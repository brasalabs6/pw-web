import {test} from 'node:test';
import assert from 'node:assert/strict';
import {PwApi,ApiError} from '../../packages/api-sdk/src/index.ts';

const api=new PwApi();
function mockFetch(response:Response,check?:(init:RequestInit|undefined)=>void){
  const old=globalThis.fetch;
  globalThis.fetch=async(_input,init)=>{check?.(init);return response};
  return()=>{globalThis.fetch=old};
}
const realm={id:'00000000-0000-4000-8000-0123456789ab',slug:'pwbr',name:'PWBR',
  description:'test',status:'offline',population:null,defaultChannel:'stable'};
test('catalog validates returned status and UUID; sends cookies',async()=>{
  const restore=mockFetch(Response.json({realms:[realm]}),init=>{
    assert.equal(init?.credentials,'include');assert.equal(init?.headers?.['Accept' as keyof HeadersInit],'application/json');
  });
  try{assert.equal((await api.realms()).realms[0].name,'PWBR')}finally{restore()}
});
test('catalog rejects forged response fields and unknown status',async()=>{
  const restore=mockFetch(Response.json({realms:[{...realm,status:'deployed'}]}));
  try{await assert.rejects(()=>api.realms())}finally{restore()}
});
test('release preflight rejects malformed digest before update',async()=>{
  const release={schema:2,realmId:realm.id,channel:'stable',pairingId:realm.id,
    client:{version:'153',releaseDigest:'X'.repeat(64),manifestUrl:'https://example.invalid/a'},
    server:{version:'153',releaseDigest:'a'.repeat(64),deploymentId:realm.id},
    assetBaseUrl:'https://example.invalid/assets'};
  const restore=mockFetch(Response.json({release}));
  try{await assert.rejects(()=>api.resolveRelease(realm.id))}finally{restore()}
});
test('API returns typed HTTP errors with status',async()=>{
  const restore=mockFetch(Response.json({error:'No supported release'},{status:409}));
  try{await assert.rejects(()=>api.resolveRelease(realm.id),
    (e:any)=>e instanceof ApiError&&e.status===409)}finally{restore()}
});
test('never transparently retries POST side effects',async()=>{
  let called=0;
  const old=globalThis.fetch;
  globalThis.fetch=async()=>{called++;throw new Error('offline')};
  try{await assert.rejects(()=>api.launch(realm.id,realm.id,'a'.repeat(64)));assert.equal(called,1)}finally{globalThis.fetch=old}
});
test('204 responses do not produce fake success JSON',async()=>{
  const restore=mockFetch(new Response(null,{status:204}));
  try{assert.equal(await api.request('/v1/admin/empty'),undefined)}finally{restore()}
});
test('SDK rejects non-versioned and remote URL paths',async()=>{
  await assert.rejects(()=>api.request('https://evil.invalid/steal'),(e:any)=>e instanceof ApiError&&e.status===400);
});
