import {test} from 'node:test';
import assert from 'node:assert/strict';
import {generateKeyPairSync,sign,createHash} from 'node:crypto';
import {canonicalJson,verifyManifest} from '../../packages/update-kit/src/index.ts';
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
