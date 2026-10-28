#!/usr/bin/env node
// Trusted release publisher only; never commit private signing keys.
import {readFileSync,writeFileSync} from 'node:fs';
import {createPrivateKey,createPublicKey,createHash,sign} from 'node:crypto';
function canonical(value) {
 if(value===null)return 'null';
 if(typeof value==='string'||typeof value==='boolean')return JSON.stringify(value);
 if(typeof value==='number'){if(!Number.isSafeInteger(value))throw Error('Noninteger');return String(value)}
 if(Array.isArray(value))return '['+value.map(canonical).join(',')+']';
 if(!value||typeof value!=='object')throw Error('Invalid JSON');
 return '{'+Object.keys(value).sort().map(k=>JSON.stringify(k)+':'+canonical(value[k])).join(',')+'}';
}
const [input,output]=process.argv.slice(2);
if(!input||!output||!process.env.RELEASE_ED25519_PRIVATE_KEY_PEM||!process.env.RELEASE_KEY_ID){
 console.error('Usage: RELEASE_ED25519_PRIVATE_KEY_PEM=... RELEASE_KEY_ID=... node sign-client-v2.mjs unsigned.json signed.json');process.exit(2);
}
const raw=JSON.parse(readFileSync(input,'utf8'));
if(raw.schema!==2||!Array.isArray(raw.files)||!raw.entry||!raw.clientVersion||!raw.assetSetDigest)
 throw Error('Invalid unsigned manifest');
if(raw.signature||raw.releaseDigest)throw Error('Expected unsigned manifest');
const digest=createHash('sha256').update(canonical(raw)).digest('hex');
const privateKey=createPrivateKey(process.env.RELEASE_ED25519_PRIVATE_KEY_PEM.replace(/\\n/g,'\n'));
const signature=sign(null,Buffer.from('pw-client-release-v2\n'+digest),privateKey).toString('base64');
const der=createPublicKey(privateKey).export({type:'spki',format:'der'});
const key=Buffer.from(der).subarray(-32).toString('base64');
writeFileSync(output,canonical({...raw,releaseDigest:digest,signature:{keyId:process.env.RELEASE_KEY_ID,value:signature}})+'\n',{flag:'wx',mode:0o600});
console.log(JSON.stringify({releaseDigest:digest,publicKeyBase64:key,keyId:process.env.RELEASE_KEY_ID}));
