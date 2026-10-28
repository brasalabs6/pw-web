import assert from 'node:assert/strict';
import pg from 'pg';
const API=process.env.API_PUBLIC_URL||'http://127.0.0.1:8787';
const mail='qa-'+Date.now()+'@example.invalid';
const json=async(path,method='GET',body,cookie,headers={})=>{
 const r=await fetch(API+path,{method,headers:{...(body?{'Content-Type':'application/json'}:{}),
   ...(cookie?{Cookie:cookie}:{}),...headers},body:body?JSON.stringify(body):undefined});
 let data;try{data=await r.json()}catch{data={}};
 return {status:r.status,data,cookie:r.headers.get('set-cookie')?.split(';')[0]};
};
for(let i=0;i<30;i++){try{const r=await fetch(API+'/readyz');if(r.ok)break}catch{} await new Promise(r=>setTimeout(r,500))}
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
r=await json('/v1/launcher/sessions','POST',{realmId},cookie);
assert.equal(r.status,409,'Must refuse gameplay when backend is absent');
r=await json('/v1/admin/previews','POST',{kind:'client',clientCommit:'0'.repeat(40),ttlHours:1},cookie);
assert.equal(r.status,422,'Idempotency-Key is required');
r=await json('/v1/auth/logout','POST',undefined,cookie);assert.equal(r.status,200);
r=await json('/v1/auth/me','GET',undefined,cookie);assert.equal(r.status,401);
console.log('API smoke passed: identity, RBAC, realm, session refusal and logout');
