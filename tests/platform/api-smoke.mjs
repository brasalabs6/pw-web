import assert from 'node:assert/strict';
import pg from 'pg';
import {createPrivateKey,sign} from 'node:crypto';
const API=process.env.API_PUBLIC_URL||'http://127.0.0.1:8787';
const mail='qa-'+Date.now()+'@example.invalid';
const json=async(path,method='GET',body,cookie,headers={})=>{
 const r=await fetch(API+path,{method,headers:{...(body?{'Content-Type':'application/json'}:{}),
   ...(cookie?{Cookie:cookie}:{}),...headers},body:body?JSON.stringify(body):undefined});
 let data;try{data=await r.json()}catch{data={}};
 return {status:r.status,data,cookie:r.headers.get('set-cookie')?.split(';')[0]};
};
for(let i=0;i<30;i++){try{const r=await fetch(API+'/readyz');if(r.ok)break}catch{} await new Promise(r=>setTimeout(r,500))}
// Browser preflight: allow explicit trusted origins and deny unknown origins.
let preflight=await fetch(API+'/v1/catalog/realms',{method:'OPTIONS',
  headers:{Origin:'http://localhost:5173','Access-Control-Request-Method':'POST'}});
assert.equal(preflight.status,204);
assert.equal(preflight.headers.get('Access-Control-Allow-Origin'),'http://localhost:5173');
preflight=await fetch(API+'/v1/catalog/realms',{method:'OPTIONS',
  headers:{Origin:'https://untrusted.example.invalid','Access-Control-Request-Method':'POST'}});
assert.equal(preflight.status,403);
let r=await json('/v1/auth/register','POST',{email:mail,password:'A-reasonably-long-test-password-123!',displayName:'Platform QA'});
assert.equal(r.status,201);
r=await json('/v1/auth/login','POST',{email:mail,password:'A-reasonably-long-test-password-123!'});
assert.equal(r.status,200);assert.ok(r.cookie);const cookie=r.cookie;
r=await json('/v1/auth/me','GET',undefined,cookie);assert.equal(r.status,200);
r=await json('/v1/admin/overview','GET',undefined,cookie);assert.equal(r.status,403);
const db=new pg.Pool({connectionString:process.env.DATABASE_URL});
try{await db.query('UPDATE users SET role=$1 WHERE email=$2',['admin',mail])}finally{await db.end()}
const slug='qa-'+Date.now();
r=await json('/v1/admin/realms','POST',{slug,name:'Test realm',description:'Not playable'},cookie);
assert.equal(r.status,201,JSON.stringify(r.data));const realmId=r.data.realm.id;
r=await json('/v1/catalog/realms');
assert.equal(r.status,200);assert.ok(r.data.realms.some(x=>x.id===realmId&&x.status==='offline'));
r=await json('/v1/launcher/releases','POST',{realmId},cookie);
assert.equal(r.status,409,'Must refuse gameplay when backend is absent');
r=await json('/v1/admin/previews','POST',{kind:'client',clientCommit:'0'.repeat(40),ttlHours:1},cookie);
assert.equal(r.status,422,'Idempotency-Key is required');
// Verify durable idempotency across retries and reject same key with modified payload.
const jobKey='smoke-preview-'+Date.now();
const previewBody={kind:'client',clientCommit:'0'.repeat(40),ttlHours:1};
const first=await json('/v1/admin/previews','POST',previewBody,cookie,{'Idempotency-Key':jobKey});
assert.equal(first.status,202,JSON.stringify(first.data));
const retry=await json('/v1/admin/previews','POST',previewBody,cookie,{'Idempotency-Key':jobKey});
assert.equal(retry.status,202,JSON.stringify(retry.data));
assert.equal(first.data.operation.id,retry.data.operation.id);
const conflict=await json('/v1/admin/previews','POST',{...previewBody,ttlHours:2},cookie,{'Idempotency-Key':jobKey});
assert.equal(conflict.status,409,'idempotency key cannot be reused with changed input');

// Concurrent approval must be serialized. Only one draft→validated transition succeeds.
const imported=await json('/v1/admin/releases/clients','POST',{
  version:'qa-approval',digest:'c'.repeat(64),artifactUrl:'https://example.invalid/client/approval.json',
  protocolProfile:'qa-profile',dataProfile:'qa-data'
},cookie);
assert.equal(imported.status,201,JSON.stringify(imported.data));
const releasePath='/v1/admin/releases/client/'+imported.data.release.id+'/status';
const approvals=await Promise.all([
  json(releasePath,'POST',{status:'validated',evidenceUrl:'https://example.invalid/evidence'},cookie),
  json(releasePath,'POST',{status:'validated',evidenceUrl:'https://example.invalid/evidence'},cookie)
]);
assert.deepEqual(approvals.map(x=>x.status).sort(),[200,409],
  'SELECT FOR UPDATE must prevent repeated release approval');

// This is a SYNTHETIC contract fixture, NOT proof of a running PW server.
// Record matching releases and a deployment, then assert a signed health attestation
// is required before the API releases any playable-looking bootstrap.
const fixtureDb=new pg.Pool({connectionString:process.env.DATABASE_URL});
let deploymentId, pairingId;
const clientDigest='a'.repeat(64);
try{
  const client=await fixtureDb.query("INSERT INTO client_releases(version,digest,artifact_url,protocol_profile,data_profile,status) VALUES('qa-client',$1,'https://example.invalid/client.json','pw153-proto','pw153-data','published') RETURNING id",[clientDigest]);
  const server=await fixtureDb.query("INSERT INTO server_releases(version,digest,artifact_url,protocol_profile,data_profile,status) VALUES('qa-server',$1,'https://example.invalid/server.json','pw153-proto','pw153-data','published') RETURNING id",['b'.repeat(64)]);
  const pairing=await fixtureDb.query("INSERT INTO pairings(client_release_id,server_release_id,protocol_profile,data_profile,status,evidence_url) VALUES($1,$2,'pw153-proto','pw153-data','supported','https://example.invalid/fixture') RETURNING id",[client.rows[0].id,server.rows[0].id]);
  pairingId=pairing.rows[0].id;
  await fixtureDb.query("UPDATE realm_channels SET pairing_id=$2,revision=revision+1 WHERE realm_id=$1 AND name='stable'",[realmId,pairingId]);
  await fixtureDb.query("UPDATE realms SET status='online' WHERE id=$1",[realmId]);
  const d=await fixtureDb.query("INSERT INTO deployments(realm_id,server_release_id,gateway_url,asset_base_url,state) VALUES($1,$2,'wss://example.invalid/connect','https://example.invalid/assets/','pending') RETURNING id",[realmId,server.rows[0].id]);
  deploymentId=d.rows[0].id;
}finally{await fixtureDb.end()}
// CAS of realm channel revision must prevent lost updates under concurrent pin.
const pins=await Promise.all([
  json('/v1/admin/realms/'+realmId+'/channels/stable/pin','POST',{pairingId,expectedRevision:1},cookie),
  json('/v1/admin/realms/'+realmId+'/channels/stable/pin','POST',{pairingId,expectedRevision:1},cookie)
]);
assert.deepEqual(pins.map(x=>x.status).sort(),[200,409],
  'channel pin must have exactly one winning revision');
r=await json('/v1/launcher/releases','POST',{realmId},cookie);
assert.equal(r.status,409,'Pending fixture deployment cannot be launched');
const checkedAt=new Date().toISOString();
const evidenceUrl='https://example.invalid/gameplay-probe/qa-only';
const attestation={checkedAt,daemonsReady:true,gsReady:true,loginAndEnterWorldPassed:true,evidenceUrl};
const payload=['pw-gameplay-health-v1',deploymentId,checkedAt,'true','true','true',evidenceUrl].join('\n');
const key=createPrivateKey({key:Buffer.from(process.env.HEALTH_PROOF_PRIVATE_KEY_PKCS8_B64,'base64'),format:'der',type:'pkcs8'});
const signature=sign(null,Buffer.from(payload),key).toString('base64');
const healthUrl='/v1/internal/deployments/'+deploymentId+'/health';
const agentBearer={Authorization:'Bearer '+process.env.AGENT_SHARED_TOKEN};
r=await json(healthUrl,'POST',{...attestation,signature:'AAAA'},undefined,agentBearer);
assert.equal(r.status,403,'Forged health signature must fail');
r=await json(healthUrl,'POST',{...attestation,signature},undefined,agentBearer);
assert.equal(r.status,200,'Signed fresh attestation accepted for synthetic fixture');
const ready=await json('/v1/launcher/releases','POST',{realmId},cookie);
assert.equal(ready.status,200,JSON.stringify(ready.data));
assert.equal(ready.data.release.client.releaseDigest,clientDigest);
assert.equal(ready.data.release.pairingId,pairingId);
r=await json('/v1/launcher/sessions','POST',{realmId,expectedPairingId:pairingId,expectedClientDigest:'0'.repeat(64)},cookie);
assert.equal(r.status,409,'Client digest change after update rejected');
r=await json('/v1/launcher/sessions','POST',{realmId,expectedPairingId:pairingId,expectedClientDigest:clientDigest},cookie);
assert.equal(r.status,200,JSON.stringify(r.data));
const ticket=r.data.bootstrap.gateway.ticket;
const gatewayBearer={Authorization:'Bearer '+process.env.GATEWAY_CONTROL_TOKEN};
const outcomes=await Promise.all([
  json('/v1/internal/tickets/consume','POST',{ticket,deploymentId},undefined,gatewayBearer),
  json('/v1/internal/tickets/consume','POST',{ticket,deploymentId},undefined,gatewayBearer)
]);
assert.deepEqual(outcomes.map(x=>x.status).sort(),[200,401],
  'exactly one concurrent consumer may use a GNET ticket');
r=await json('/v1/internal/tickets/consume','POST',{ticket,deploymentId},undefined,gatewayBearer);
assert.equal(r.status,401,'ticket replay must fail');
// Durable brute-force throttling: the ninth attempt is denied across requests.
const blockedEmail='blocked-'+Date.now()+'@example.invalid';
for(let i=0;i<9;i++){
  const invalid=await json('/v1/auth/login','POST',{email:blockedEmail,password:'wrong'});
  assert.equal(invalid.status,i<8?401:429,'login attempt '+i);
}
// Logging out must revoke an unconsumed game ticket by FK CASCADE.
r=await json('/v1/launcher/sessions','POST',{realmId,expectedPairingId:pairingId,expectedClientDigest:clientDigest},cookie);
assert.equal(r.status,200,'synthetic ready fixture can issue ticket');
const outstandingTicket=r.data.bootstrap.gateway.ticket;
r=await json('/v1/auth/logout','POST',undefined,cookie);assert.equal(r.status,200);
r=await json('/v1/auth/me','GET',undefined,cookie);assert.equal(r.status,401);
r=await json('/v1/internal/tickets/consume','POST',{ticket:outstandingTicket,deploymentId},undefined,gatewayBearer);
assert.equal(r.status,401,'logout must revoke outstanding tickets');
console.log('API smoke passed: identity, atomic realm, idempotency, signed health, release pin, ticket replay and logout (synthetic ready fixture only)');
