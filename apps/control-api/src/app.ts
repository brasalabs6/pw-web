import {Hono} from 'hono';
import {getCookie,setCookie,deleteCookie} from 'hono/cookie';
import {z} from 'zod';
import {connectDb,type Db} from '@pw/database';
import {registerInput,loginInput,launcherSessionInput,launchReleaseInput,createRealmInput,updateRealmInput,
  createPreviewInput,patchChannelInput,pairingCreateInput,releaseCreateInput,compatiblePair} from '@pw/contracts';
import {randomToken,passwordHash,verifyPassword,sha256,safeOrigin} from './security.ts';

type User={id:string;email:string;role:'player'|'operator'|'admin';display_name:string;session_id:string};
type Env={Variables:{user:User|null}};
const app=new Hono<Env>();
export const db=connectDb();
const cookieOpts={httpOnly:true,secure:process.env.NODE_ENV==='production',sameSite:'Strict' as const,path:'/',maxAge:7*86400};
const fail=(c:any,status:400|401|403|404|409|422|503,message:string)=>c.json({error:message},status);
const json=async <T extends z.ZodType>(c:any,schema:T):Promise<z.output<T>|null>=>{try{return schema.parse(await c.req.json())}catch{return null}};

app.use('/v1/*',async(c,next)=>{
  const origin=c.req.header('origin')??null;
  if(c.req.method!=='GET'&&c.req.method!=='HEAD'&&!safeOrigin(origin)) return fail(c,403,'Origin not allowed');
  c.header('Cache-Control','no-store');
  c.header('X-Content-Type-Options','nosniff');
  const token=getCookie(c,'pw_session');
  let user:User|null=null;
  if(token){
    const q=await db.query(
      'SELECT u.id,u.email,u.role,u.display_name,s.id AS session_id FROM web_sessions s JOIN users u ON s.user_id=u.id WHERE s.token_sha256=$1 AND s.expires_at>now()',
      [sha256(token)]);
    user=q.rows[0]??null;
  }
  c.set('user',user);
  await next();
});
app.onError((error,c)=>{
  console.error('[control-api]',error instanceof Error?error.message:'unknown');
  return c.json({error:'Internal server error'},500);
});
const mustUser=(c:any)=>c.get('user') as User|null;
const mustAdmin=(c:any)=>{const u=mustUser(c);return u&&['admin','operator'].includes(u.role)?u:null};
const audit=async (db:Db,actor:string,action:string,resource:string,details:unknown={})=>{
  await db.query('INSERT INTO audit_events(actor_id,action,resource,details) VALUES($1,$2,$3,$4)',[actor,action,resource,JSON.stringify(details)]);
};

// Shared durable rate limiting; works across control-api processes and fails closed.
const authThrottle=async(action:'login'|'register', email:string) => {
  const key=sha256('pw-auth-v1:'+action+':'+email.toLowerCase());
  const q=await db.query(`INSERT INTO auth_rate_buckets(key_sha256,attempts,bucket_start)
    VALUES($1,1,now()) ON CONFLICT(key_sha256) DO UPDATE SET
    attempts=CASE WHEN auth_rate_buckets.bucket_start < now()-interval '15 minutes'
      THEN 1 ELSE auth_rate_buckets.attempts+1 END,
    bucket_start=CASE WHEN auth_rate_buckets.bucket_start < now()-interval '15 minutes'
      THEN now() ELSE auth_rate_buckets.bucket_start END
    RETURNING attempts`,[key]);
  return {key, allowed:q.rows[0].attempts <= (action==='login'?8:3)};
};

app.get('/healthz',c=>c.json({status:'alive',service:'control-api'}));
app.get('/readyz',async c=>{try{await db.query('SELECT 1');return c.json({status:'ready'})}catch{return fail(c,503,'database unavailable')}});
app.post('/v1/auth/register',async c=>{
  const input=await json(c,registerInput);if(!input)return fail(c,422,'Invalid registration input');
  const throttle=await authThrottle('register',input.email);
  if(!throttle.allowed)return c.json({error:'Registration temporarily restricted'},429);
  const exists=await db.query('SELECT 1 FROM users WHERE email=$1',[input.email]);if(exists.rowCount)return fail(c,409,'Email already registered');
  const q=await db.query(
    'INSERT INTO users(email,display_name,password_hash) VALUES($1,$2,$3) RETURNING id,email,display_name,role',
    [input.email,input.displayName,passwordHash(input.password)]);
  return c.json({user:q.rows[0],message:'Web identity created; game account provisioning is not enabled yet'},201);
});
app.post('/v1/auth/login',async c=>{
  const input=await json(c,loginInput);if(!input)return fail(c,422,'Invalid credentials');
  const throttle=await authThrottle('login',input.email);
  if(!throttle.allowed)return c.json({error:'Too many login attempts. Try again later.'},429);
  const q=await db.query('SELECT * FROM users WHERE email=$1',[input.email.toLowerCase()]);
  const user=q.rows[0];if(!user||!verifyPassword(input.password,user.password_hash))return fail(c,401,'Invalid credentials');
  const token=randomToken();
  await db.query('INSERT INTO web_sessions(user_id,token_sha256,expires_at) VALUES($1,$2,now()+interval \'7 days\')',[user.id,sha256(token)]);
  await db.query('DELETE FROM auth_rate_buckets WHERE key_sha256=$1',[throttle.key]);
  setCookie(c,'pw_session',token,cookieOpts);
  return c.json({user:{id:user.id,email:user.email,displayName:user.display_name,role:user.role}});
});
app.post('/v1/auth/logout',async c=>{
  const user=mustUser(c);if(user)await db.query('DELETE FROM web_sessions WHERE id=$1',[user.session_id]);
  deleteCookie(c,'pw_session',{path:'/'});return c.json({ok:true});
});
app.get('/v1/auth/me',c=>{
  const u=mustUser(c);return u?c.json({user:{id:u.id,email:u.email,displayName:u.display_name,role:u.role}}):fail(c,401,'Sign in required');
});
app.get('/v1/catalog/realms',async c=>{
  const q=await db.query(
    `SELECT r.id,r.slug,r.name,r.description,r.population,
      r.default_channel AS "defaultChannel",
      CASE WHEN r.status='online' AND EXISTS (
        SELECT 1 FROM deployments d JOIN realm_channels ch ON ch.realm_id=r.id AND ch.name=r.default_channel
        JOIN pairings p ON ch.pairing_id=p.id
        WHERE d.realm_id=r.id AND d.server_release_id=p.server_release_id
          AND d.state='ready' AND d.health_checked_at > now()-interval '30 seconds'
          AND p.status='supported'
      ) THEN 'online'
      WHEN r.status='online' THEN 'starting'
      ELSE r.status END AS status
      FROM realms r ORDER BY r.name LIMIT 200`);
  return c.json({realms:q.rows});
});
// Never claim a realm online solely based on a database flag: return starting until heartbeat route is verified.
app.get('/v1/catalog/realms/:id',async c=>{
  const q=await db.query('SELECT id,slug,name,description,status,default_channel FROM realms WHERE id=$1',[c.req.param('id')]);
  if(!q.rowCount)return fail(c,404,'Realm not found');
  return c.json({realm:q.rows[0]});
});
type PinnedSelection={
  pairing_id:string; client_version:string;client_digest:string;client_manifest_url:string;
  server_version:string;server_digest:string;deployment_id:string;gateway_url:string;asset_base_url:string;
  status:string;deployment_state:string;pair_status:string;client_status:string;server_status:string;
  health_checked_at:Date|null;
};
// Returns null if the selected realm is not compatible and healthy; never falls back to a different client.
const resolvePinnedRelease=async(realmId:string,channel:'stable'):Promise<PinnedSelection|null>=>{
  const q=await db.query(
    `SELECT r.status,r.name,rc.pairing_id,p.status AS pair_status,p.protocol_profile,p.data_profile,
      cr.version AS client_version,cr.digest AS client_digest,cr.artifact_url AS client_manifest_url,cr.status AS client_status,
      sr.version AS server_version,sr.digest AS server_digest,sr.status AS server_status,
      d.id AS deployment_id,d.gateway_url,d.asset_base_url,d.state AS deployment_state,d.health_checked_at
    FROM realms r JOIN realm_channels rc ON rc.realm_id=r.id AND rc.name=$2
    JOIN pairings p ON p.id=rc.pairing_id
    JOIN client_releases cr ON cr.id=p.client_release_id
    JOIN server_releases sr ON sr.id=p.server_release_id
    JOIN deployments d ON d.realm_id=r.id AND d.server_release_id=sr.id
    WHERE r.id=$1 ORDER BY d.updated_at DESC LIMIT 1`,[realmId,channel]);
  const x=q.rows[0] as PinnedSelection|undefined;
  if(!x || x.status!=='online'||x.deployment_state!=='ready'||x.pair_status!=='supported'||
     x.client_status!=='published'||x.server_status!=='published')return null;
  if(!x.health_checked_at||Date.now()-new Date(x.health_checked_at).getTime()>30000)return null;
  if(!x.gateway_url.startsWith('wss://')||!x.asset_base_url.startsWith('https://')||
     !x.client_manifest_url.startsWith('https://'))return null;
  return x;
};
const releaseResponse=(x:PinnedSelection,realmId:string,channel:string)=>({
  schema:2 as const,realmId,channel,pairingId:x.pairing_id,
  client:{version:x.client_version,releaseDigest:x.client_digest,manifestUrl:x.client_manifest_url},
  server:{version:x.server_version,releaseDigest:x.server_digest,deploymentId:x.deployment_id},
  assetBaseUrl:x.asset_base_url
});
// Prepare/verify the client BEFORE requesting a 90-second one-use GNET ticket.
app.post('/v1/launcher/releases',async c=>{
  if(!mustUser(c))return fail(c,401,'Sign in required');
  const input=await json(c,launchReleaseInput);if(!input)return fail(c,422,'Invalid realm');
  const x=await resolvePinnedRelease(input.realmId,input.channel);
  if(!x)return fail(c,409,'Realm not ready or no supported client release');
  return c.json({release:releaseResponse(x,input.realmId,input.channel)});
});
app.post('/v1/launcher/sessions',async c=>{
  const u=mustUser(c);if(!u)return fail(c,401,'Sign in required');
  const input=await json(c,launcherSessionInput);if(!input)return fail(c,422,'Invalid launcher request');
  const x=await resolvePinnedRelease(input.realmId,input.channel);
  if(!x)return fail(c,409,'Realm not ready or no supported client release');
  if(x.pairing_id!==input.expectedPairingId||x.client_digest!==input.expectedClientDigest)
    return fail(c,409,'Client release changed during update; reverify new release');
  const token=randomToken();
  await db.query(
    'INSERT INTO game_tickets(token_sha256,session_id,realm_id,deployment_id,pairing_id,expires_at) VALUES($1,$2,$3,$4,$5,now()+interval \'90 seconds\')',
    [sha256(token),u.session_id,input.realmId,x.deployment_id,x.pairing_id]);
  return c.json({bootstrap:{...releaseResponse(x,input.realmId,input.channel),
    gateway:{url:x.gateway_url,ticket:token,ticketExpiresAt:new Date(Date.now()+90000).toISOString()}}});
});
// Gateway is private-only, uses a separately configured bearer credential and consumes tickets once.
app.post('/v1/internal/tickets/consume',async c=>{
  const header=c.req.header('Authorization');
  const expected=process.env.GATEWAY_CONTROL_TOKEN;
  if(!expected||!header||header!==`Bearer ${expected}`)return fail(c,403,'Forbidden');
  const body=await json(c,z.object({ticket:z.string().min(32),deploymentId:z.string().uuid()}));
  if(!body)return fail(c,422,'Invalid ticket');
  const q=await db.query(
    `UPDATE game_tickets SET consumed_at=now() WHERE token_sha256=$1 AND deployment_id=$2
       AND expires_at>now() AND consumed_at IS NULL RETURNING realm_id,pairing_id`,
    [sha256(body.ticket),body.deploymentId]);
  if(!q.rowCount)return fail(c,401,'Ticket invalid, expired or already used');
  return c.json({ok:true,realmId:q.rows[0].realm_id,pairingId:q.rows[0].pairing_id});
});

app.get('/v1/admin/overview',async c=>{
  if(!mustAdmin(c))return fail(c,403,'Operator required');
  const [realms,clientReleases,serverReleases,jobs]=await Promise.all([
    db.query('SELECT id,slug,name,status,population FROM realms ORDER BY name'),
    db.query('SELECT id,version,digest,status FROM client_releases ORDER BY created_at DESC LIMIT 100'),
    db.query('SELECT id,version,digest,status FROM server_releases ORDER BY created_at DESC LIMIT 100'),
    db.query('SELECT id,kind,state,payload,result,created_at FROM operation_jobs ORDER BY created_at DESC LIMIT 100')
  ]);
  return c.json({realms:realms.rows,clientReleases:clientReleases.rows,serverReleases:serverReleases.rows,jobs:jobs.rows});
});
app.post('/v1/admin/realms',async c=>{
  const u=mustAdmin(c);if(!u)return fail(c,403,'Operator required');
  const input=await json(c,createRealmInput);if(!input)return fail(c,422,'Invalid realm');
  const tx=await db.connect();
  try{
    await tx.query('BEGIN');
    const q=await tx.query(
      'INSERT INTO realms(slug,name,description,default_channel) VALUES($1,$2,$3,$4) ON CONFLICT(slug) DO NOTHING RETURNING *',
      [input.slug,input.name,input.description,input.defaultChannel]);
    if(!q.rowCount){await tx.query('ROLLBACK');return fail(c,409,'Realm slug already exists')}
    await tx.query('INSERT INTO realm_channels(realm_id,name) VALUES($1,$2)',[q.rows[0].id,input.defaultChannel]);
    await tx.query('INSERT INTO audit_events(actor_id,action,resource,details) VALUES($1,$2,$3,$4)',
      [u.id,'realm.create',q.rows[0].id,JSON.stringify({slug:input.slug})]);
    await tx.query('COMMIT');
    return c.json({realm:q.rows[0]},201);
  }catch(e){await tx.query('ROLLBACK');throw e}finally{tx.release()}
});
app.patch('/v1/admin/realms/:id',async c=>{
  const u=mustAdmin(c);if(!u)return fail(c,403,'Operator required');
  const input=await json(c,updateRealmInput);if(!input)return fail(c,422,'Invalid realm patch');
  const q=await db.query('UPDATE realms SET name=COALESCE($2,name),description=COALESCE($3,description),status=COALESCE($4,status) WHERE id=$1 RETURNING *',
    [c.req.param('id'),input.name??null,input.description??null,input.status??null]);
  if(!q.rowCount)return fail(c,404,'Realm not found');
  await audit(db,u.id,'realm.update',c.req.param('id'),input);
  return c.json({realm:q.rows[0]});
});
for (const [kind,table] of [['clients','client_releases'],['servers','server_releases']] as const) {
  app.post(`/v1/admin/releases/${kind}`,async c=>{
    const u=mustAdmin(c);if(!u)return fail(c,403,'Operator required');
    const input=await json(c,releaseCreateInput);if(!input)return fail(c,422,'Invalid release input');
    const q=await db.query(`INSERT INTO ${table}(version,digest,artifact_url,protocol_profile,data_profile,status)
      VALUES($1,$2,$3,$4,$5,$6) ON CONFLICT(digest) DO NOTHING RETURNING *`,
      [input.version,input.digest,input.artifactUrl,input.protocolProfile,input.dataProfile,'draft']);
    if(!q.rowCount)return fail(c,409,'Release digest already registered');
    await audit(db,u.id,`release.${kind}.import`,q.rows[0].id,{digest:input.digest});
    return c.json({release:q.rows[0]},201);
  });
}
app.get('/v1/admin/pairings',async c=>{
  if(!mustAdmin(c))return fail(c,403,'Operator required');
  const q=await db.query('SELECT * FROM pairings ORDER BY id LIMIT 200');
  return c.json({pairings:q.rows});
});
app.get('/v1/admin/channels',async c=>{
  if(!mustAdmin(c))return fail(c,403,'Operator required');
  const q=await db.query('SELECT ch.realm_id,r.name AS realm_name,ch.name,ch.pairing_id,ch.revision FROM realm_channels ch JOIN realms r ON r.id=ch.realm_id ORDER BY r.name,ch.name');
  return c.json({channels:q.rows});
});
app.post('/v1/admin/pairings',async c=>{
  const u=mustAdmin(c);if(!u)return fail(c,403,'Operator required');
  const input=await json(c,pairingCreateInput);if(!input)return fail(c,422,'Invalid pairing');
  const r=await db.query('SELECT protocol_profile,data_profile FROM client_releases WHERE id=$1',[input.clientReleaseId]);
  const s=await db.query('SELECT protocol_profile,data_profile FROM server_releases WHERE id=$1',[input.serverReleaseId]);
  if(!r.rowCount||!s.rowCount)return fail(c,404,'Missing release');
  if(!compatiblePair(r.rows[0],s.rows[0])||input.protocolProfile!==r.rows[0].protocol_profile||
     input.dataProfile!==r.rows[0].data_profile)return fail(c,409,'Client/server protocol or data profiles differ');
  if(input.status==='supported' && u.role!=='admin')return fail(c,403,'Admin required to approve supported pairing');
  if(input.status==='supported' && !input.evidenceUrl)return fail(c,422,'Proof URL required for supported pairing');
  const q=await db.query(
    'INSERT INTO pairings(client_release_id,server_release_id,protocol_profile,data_profile,status,evidence_url) VALUES($1,$2,$3,$4,$5,$6) ON CONFLICT(client_release_id,server_release_id) DO NOTHING RETURNING *',
    [input.clientReleaseId,input.serverReleaseId,input.protocolProfile,input.dataProfile,input.status,input.evidenceUrl??null]);
  if(!q.rowCount)return fail(c,409,'Pair already exists');
  await audit(db,u.id,'pairing.create',q.rows[0].id,{status:input.status});
  return c.json({pairing:q.rows[0]},201);
});
app.post('/v1/admin/realms/:id/channels/:name/pin',async c=>{
  const u=mustAdmin(c);if(!u)return fail(c,403,'Operator required');
  const input=await json(c,patchChannelInput);if(!input)return fail(c,422,'Invalid channel pin');
  const pair=await db.query('SELECT status FROM pairings WHERE id=$1',[input.pairingId]);
  if(pair.rows[0]?.status!=='supported')return fail(c,409,'Only proven supported pairings can be pinned');
  const q=await db.query('UPDATE realm_channels SET pairing_id=$3,revision=revision+1 WHERE realm_id=$1 AND name=$2 AND revision=$4 RETURNING *',
    [c.req.param('id'),c.req.param('name'),input.pairingId,input.expectedRevision]);
  if(!q.rowCount)return fail(c,409,'Channel missing or revision changed');
  await audit(db,u.id,'channel.pin',c.req.param('id'),{channel:c.req.param('name'),pairingId:input.pairingId});
  return c.json({channel:q.rows[0]});
});
app.post('/v1/admin/previews',async c=>{
  const u=mustAdmin(c);if(!u)return fail(c,403,'Operator required');
  const input=await json(c,createPreviewInput);if(!input)return fail(c,422,'Invalid preview');
  const key=c.req.header('Idempotency-Key');
  if(!key||key.length<12||key.length>120)return fail(c,422,'Idempotency-Key required (12-120 chars)');
  const tx=await db.connect();
  try{
    await tx.query('BEGIN');
    const inserted=await tx.query(
      `INSERT INTO operation_jobs(kind,payload,actor_id,idempotency_key)
        VALUES('preview.create',$1,$2,$3)
        ON CONFLICT(idempotency_key) DO NOTHING
        RETURNING id,kind,state,payload,created_at`,[JSON.stringify(input),u.id,key]);
    if(inserted.rowCount){
      await tx.query('INSERT INTO audit_events(actor_id,action,resource,details) VALUES($1,$2,$3,$4)',
        [u.id,'preview.request',inserted.rows[0].id,JSON.stringify(input)]);
      await tx.query('COMMIT');
      return c.json({operation:inserted.rows[0]},202);
    }
    const existing=await tx.query(
      'SELECT id,kind,state,payload,created_at,actor_id FROM operation_jobs WHERE idempotency_key=$1',[key]);
    const x=existing.rows[0];
    if(!x || x.actor_id!==u.id || JSON.stringify(x.payload)!==JSON.stringify(input)){
      await tx.query('ROLLBACK');return fail(c,409,'Idempotency key already used for another operation');
    }
    await tx.query('COMMIT');
    return c.json({operation:{id:x.id,kind:x.kind,state:x.state,payload:x.payload,created_at:x.created_at}},202);
  }catch(e){await tx.query('ROLLBACK');throw e}finally{tx.release()}
});
app.get('/v1/admin/jobs',async c=>{
  if(!mustAdmin(c))return fail(c,403,'Operator required');
  const q=await db.query('SELECT id,kind,state,payload,result,created_at,updated_at FROM operation_jobs ORDER BY created_at DESC LIMIT 100');
  return c.json({jobs:q.rows});
});

// Deployment enrollment is administrative; readiness can only be asserted by a trusted probe.
app.get('/v1/admin/deployments',async c=>{
  if(!mustAdmin(c))return fail(c,403,'Operator required');
  const q=await db.query('SELECT * FROM deployments ORDER BY updated_at DESC LIMIT 200');
  return c.json({deployments:q.rows});
});
app.post('/v1/admin/deployments',async c=>{
  const u=mustAdmin(c);if(!u)return fail(c,403,'Operator required');
  const input=await json(c,z.object({
    realmId:z.string().uuid(),serverReleaseId:z.string().uuid(),
    gatewayUrl:z.url().startsWith('wss://'),assetBaseUrl:z.url().startsWith('https://')
  }));if(!input)return fail(c,422,'Invalid deployment');
  const rel=await db.query('SELECT status FROM server_releases WHERE id=$1',[input.serverReleaseId]);
  if(rel.rows[0]?.status!=='published')return fail(c,409,'Server release must be published');
  const q=await db.query(
    "INSERT INTO deployments(realm_id,server_release_id,gateway_url,asset_base_url,state) VALUES($1,$2,$3,$4,'pending') RETURNING *",
    [input.realmId,input.serverReleaseId,input.gatewayUrl,input.assetBaseUrl]);
  await audit(db,u.id,'deployment.register',q.rows[0].id,{realmId:input.realmId});
  return c.json({deployment:q.rows[0]},201);
});
app.post('/v1/admin/releases/:kind/:id/status',async c=>{
  const u=mustUser(c);if(!u||u.role!=='admin')return fail(c,403,'Admin approval required');
  const kind=c.req.param('kind');
  const table=kind==='client'?'client_releases':kind==='server'?'server_releases':null;
  if(!table)return fail(c,404,'Unknown release kind');
  const input=await json(c,z.object({status:z.enum(['validated','published','revoked']),evidenceUrl:z.url()}));
  if(!input)return fail(c,422,'Evidence URL and status required');
  const previous=await db.query('SELECT status FROM '+table+' WHERE id=$1',[c.req.param('id')]);
  if(!previous.rowCount)return fail(c,404,'Release not found');
  const allowed:Record<string,string[]>={draft:['validated','revoked'],validated:['published','revoked'],published:['revoked'],revoked:[]};
  if(!allowed[previous.rows[0].status]?.includes(input.status))return fail(c,409,'Invalid release state transition');
  const q=await db.query('UPDATE '+table+' SET status=$2 WHERE id=$1 RETURNING *',[c.req.param('id'),input.status]);
  await audit(db,u.id,'release.'+kind+'.'+input.status,q.rows[0].id,{evidenceUrl:input.evidenceUrl});
  return c.json({release:q.rows[0]});
});
// Only trusted probes can update readiness. TCP reachability alone never implies gameplay ready.
// An external probe MUST verify login, role and entry world before sending gameplayReady=true.
app.post('/v1/internal/deployments/:id/health',async c=>{
  const expected=process.env.AGENT_SHARED_TOKEN;
  if(!expected||expected.length<32||c.req.header('Authorization')!=='Bearer '+expected)return fail(c,403,'Forbidden');
  const input=await json(c,z.object({
    daemonsReady:z.boolean(),gsReady:z.boolean(),loginAndEnterWorldPassed:z.boolean(),
    evidenceUrl:z.url()
  }));if(!input)return fail(c,422,'Invalid health proof');
  const ready=input.daemonsReady&&input.gsReady&&input.loginAndEnterWorldPassed;
  const q=await db.query(
    "UPDATE deployments SET state=$2,health_checked_at=now(),updated_at=now() WHERE id=$1 RETURNING realm_id",
    [c.req.param('id'),ready?'ready':'degraded']);
  if(!q.rowCount)return fail(c,404,'Unknown deployment');
  return c.json({ready,gameplayProof:ready,evidenceUrl:input.evidenceUrl});
});

export {app};
