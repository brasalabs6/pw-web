import {test} from 'node:test';
import assert from 'node:assert/strict';
import {generateKeyPairSync,sign,createHash} from 'node:crypto';
import {canonicalJson,verifyManifest,prepareClient,digest} from '../../packages/update-kit/src/index.ts';
const {publicKey,privateKey}=generateKeyPairSync('ed25519');
const pub=Buffer.from(publicKey.export({format:'der',type:'spki'})).subarray(-32).toString('base64');
const raw={
 schema:2,clientVersion:'1.5.3-v145',assetSetDigest:'a'.repeat(64),
 entry:{js:{url:'https://example.invalid/entry.mjs',sha256:'b'.repeat(64),bytes:100},
   wasm:{url:'https://example.invalid/client.wasm',sha256:'c'.repeat(64),bytes:200}},
 files:[{path:'entry/test.bin',url:'https://example.invalid/test.bin',sha256:'d'.repeat(64),bytes:0}]
};
const hash=createHash('sha256').update(canonicalJson(raw)).digest('hex');
const signed={...raw,releaseDigest:hash,signature:{keyId:'test-public',value:sign(null,Buffer.from('pw-client-release-v2\n'+hash),privateKey).toString('base64')}};
test('valid Ed25519 manifest and digest are accepted',async()=>{
 const verified=await verifyManifest(signed,hash,{'test-public':pub});
 assert.equal(verified.clientVersion,'1.5.3-v145');
});
test('manifest digest changed after signing is rejected',async()=>{
 await assert.rejects(()=>verifyManifest({...signed,clientVersion:'1.4.4'},hash,{'test-public':pub}),/digest mismatch/);
});
test('wrong public signing key is rejected',async()=>{
 const {publicKey:other}=generateKeyPairSync('ed25519');
 const rawOther=Buffer.from(other.export({format:'der',type:'spki'})).subarray(-32).toString('base64');
 await assert.rejects(()=>verifyManifest(signed,hash,{'test-public':rawOther}),/signature/);
});
test('missing trusted key is rejected fail-closed',async()=>{
 await assert.rejects(()=>verifyManifest(signed,hash,{}),/Untrusted signing key/);
});

test('logical asset paths reject traversal, encoded slashes and backslashes',async()=>{
  const original=signed.files[0];
  for(const path of ['../secrets','a/../b','/root','a//b','a\\b','a%2fb','a/./c']){
    await assert.rejects(()=>verifyManifest({...signed,files:[{...original,path}]},hash,{'test-public':pub}));
  }
});
test('case-colliding file paths are rejected',async()=>{
 const entry={...signed.files[0]};
 const raw2={...raw,files:[{...entry,path:'Maps/001.bin'},{...entry,path:'maps/001.bin'}]};
 const digest2=createHash('sha256').update(canonicalJson(raw2)).digest('hex');
 const signature2=sign(null,Buffer.from('pw-client-release-v2\n'+digest2),privateKey).toString('base64');
 await assert.rejects(()=>verifyManifest({...raw2,releaseDigest:digest2,signature:{keyId:'test-public',value:signature2}},digest2,{'test-public':pub}),/Duplicate/);
});

test('client update checks manifest, verifies every byte and reuses cache by hash',async()=>{
 const js=new TextEncoder().encode('export const angelicaEntry=1;');
 const wasm=new Uint8Array([0,97,115,109,1,0,0,0]); // valid minimal WASM binary
 const fakeManifestUrl='https://cdn.example.invalid/pw/client153/release.json';
 const jsURL='https://cdn.example.invalid/pw/client153/main.js';
 const wasmURL='https://cdn.example.invalid/pw/client153/main.wasm';
 const release={
   schema:2,clientVersion:'pw153-ci-fixture',assetSetDigest:'f'.repeat(64),
   entry:{
     js:{url:jsURL,bytes:js.byteLength,sha256:await digest(js)},
     wasm:{url:wasmURL,bytes:wasm.byteLength,sha256:await digest(wasm)}
   },files:[]
 };
 const releaseDigest=createHash('sha256').update(canonicalJson(release)).digest('hex');
 const signedManifest={...release,releaseDigest,signature:{
   keyId:'test-public',
   value:sign(null,Buffer.from('pw-client-release-v2\n'+releaseDigest),privateKey).toString('base64')
 }};
 const body=canonicalJson(signedManifest)+'\n';
 let assetFetches=0;
 const store=new Map<string,Response>();
 const origFetch=globalThis.fetch;
 const origCaches=globalThis.caches;
 const origLocation=globalThis.location;
 (globalThis as any).location={origin:'https://launcher.example.invalid',hostname:'launcher.example.invalid'};
 (globalThis as any).caches={open:async()=>({
   match:async(req:Request)=>store.get(req.url)?.clone(),
   put:async(req:Request,res:Response)=>{store.set(req.url,res.clone())},
   delete:async(req:Request)=>store.delete(req.url)
 })};
 (globalThis as any).fetch=async (raw:URL|string)=>{
   const url=String(raw);
   if(url===fakeManifestUrl)return new Response(body,{headers:{'Content-Type':'application/json'}});
   if(url===jsURL){assetFetches++;return new Response(js)}
   if(url===wasmURL){assetFetches++;return new Response(wasm)}
   throw new Error('Unexpected URL '+url);
 };
 try{
   const options={manifestUrl:fakeManifestUrl,expectedDigest:releaseDigest,trustedKeys:{'test-public':pub}};
   const first=await prepareClient(options);
   assert.deepEqual(first.wasm,wasm);
   assert.equal(assetFetches,2,'first launch fetches both assets');
   const second=await prepareClient(options);
   assert.deepEqual(second.js,js);
   assert.equal(assetFetches,2,'warm-cache must avoid redundant download');
   // A duplicate JSON key is rejected even if the parsed object appears harmless.
   (globalThis as any).fetch=async(raw:URL|string)=>{
     if(String(raw)===fakeManifestUrl)return new Response(body.replace('"schema":2','"schema":2,"schema":2'));
     throw new Error('Unexpected asset request after manifest corruption');
   };
   await assert.rejects(()=>prepareClient(options),/Noncanonical|duplicate-key/);
 }finally{
   globalThis.fetch=origFetch;
   (globalThis as any).caches=origCaches;
   (globalThis as any).location=origLocation;
 }
});
