import {updateManifest, type UpdateManifest} from '@pw/contracts';

const enc=new TextEncoder();
const hex=(bytes:Uint8Array)=>Array.from(bytes,b=>b.toString(16).padStart(2,'0')).join('');
export async function digest(bytes:Uint8Array):Promise<string>{
  return hex(new Uint8Array(await crypto.subtle.digest('SHA-256',bytes as BufferSource)));
}
const fromBase64=(b64:string)=>Uint8Array.from(atob(b64),c=>c.charCodeAt(0));
// Strict deterministic JSON representation for all manifest signers and consumers.
// Rejects numeric edge cases and ambiguous object keys; producer must use same algorithm.
export function canonicalJson(value:any):string{
  if(value===null)return 'null';
  if(typeof value==='string'||typeof value==='boolean')return JSON.stringify(value);
  if(typeof value==='number'){if(!Number.isSafeInteger(value))throw new Error('Noninteger manifest field');return JSON.stringify(value)}
  if(Array.isArray(value))return '['+value.map(canonicalJson).join(',')+']';
  if(typeof value!=='object')throw new Error('Invalid manifest value');
  const keys=Object.keys(value).sort();
  return '{'+keys.map(k=>JSON.stringify(k)+':'+canonicalJson(value[k])).join(',')+'}';
}
export type TrustedKeys=Record<string,string>;
export async function verifyManifest(raw:unknown, expectedDigest:string, keys:TrustedKeys):Promise<UpdateManifest>{
  const manifest=updateManifest.parse(raw);
  const {signature,releaseDigest,...signedContent}=manifest;
  const paths=new Set<string>();
  for(const file of manifest.files){
    const key=file.path.toLowerCase();
    if(paths.has(key))throw new Error('Duplicate or case-colliding asset path');
    paths.add(key);
  }
  const hash=await digest(enc.encode(canonicalJson(signedContent)));
  if(hash!==releaseDigest || hash!==expectedDigest)throw new Error('Client release digest mismatch');
  const publicKeyB64=keys[signature.keyId];
  if(!publicKeyB64)throw new Error('Untrusted signing key: '+signature.keyId);
  const key=await crypto.subtle.importKey('raw',fromBase64(publicKeyB64) as BufferSource,{name:'Ed25519'},false,['verify']);
  const ok=await crypto.subtle.verify('Ed25519',key,fromBase64(signature.value) as BufferSource,
    enc.encode('pw-client-release-v2\n'+hash) as BufferSource);
  if(!ok)throw new Error('Invalid client release signature');
  return manifest;
}
type Asset={url:string;sha256:string;bytes:number};
const shaRegex=/^[a-f0-9]{64}$/;
const MAX_ASSET_BYTES=256*1024*1024;
const MAX_MANIFEST_BYTES=2*1024*1024;
function secureUrl(raw:string,base:string,expectedOrigin?:string):URL {
  const url=new URL(raw,base);
  if(url.username||url.password||url.hash)throw new Error('Credentials/fragments are not allowed in asset URLs');
  if(url.protocol!=='https:' && !(url.hostname==='localhost' && url.protocol==='http:'))
    throw new Error('HTTPS required for update downloads');
  if(expectedOrigin && url.origin!==expectedOrigin)throw new Error('Untrusted asset origin');
  return url;
}
async function boundedRead(response:Response,maximum:number):Promise<Uint8Array>{
  if(!response.body)throw new Error('Streaming fetch response required');
  const size=response.headers.get('Content-Length');
  if(size && /^\d+$/.test(size) && Number(size)>maximum)throw new Error('Remote object exceeds declared size');
  const reader=response.body.getReader();
  const parts:Uint8Array[]=[];
  let count=0;
  try {
    while(true){
      const {done,value}=await reader.read();
      if(done)break;
      if(count+value.byteLength>maximum)throw new Error('Remote object exceeds declared size');
      parts.push(value);count+=value.byteLength;
    }
  }catch(e){await reader.cancel().catch(()=>{});throw e}
  const bytes=new Uint8Array(count);
  let offset=0;
  for(const part of parts){bytes.set(part,offset);offset+=part.byteLength}
  return bytes;
}
const cacheKey=(sha:string)=>new Request(new URL('/__pw_hash_cache__/'+sha,location.origin).toString());
async function verifiedBytes(entry:Asset,base:string,signal?:AbortSignal):Promise<Uint8Array>{
  if(!shaRegex.test(entry.sha256)||entry.bytes>MAX_ASSET_BYTES||entry.bytes<0)
    throw new Error('Invalid asset metadata');
  const baseURL=secureUrl(base,base);
  const u=secureUrl(entry.url,base,baseURL.origin);
  const store=await caches.open('pw-assets-sha256-v1');
  const cached=await store.match(cacheKey(entry.sha256));
  if(cached){
    // Object content is always verified on reuse, never trusted by URL alone.
    const advertised=cached.headers.get('Content-Length');
    if(!advertised||Number(advertised)<=entry.bytes){
      const content=await boundedRead(cached,entry.bytes);
      if(content.length===entry.bytes && await digest(content)===entry.sha256)return content;
    }
    await store.delete(cacheKey(entry.sha256));
  }
  const response=await fetch(u,{signal,credentials:'omit',mode:'cors',cache:'no-store'});
  if(!response.ok)throw new Error('Download failed '+response.status);
  if(secureUrl(response.url,u.toString()).origin!==baseURL.origin)
    throw new Error('Asset redirected to untrusted origin');
  const bytes=await boundedRead(response,entry.bytes);
  if(bytes.length!==entry.bytes || await digest(bytes)!==entry.sha256)
    throw new Error('Asset SHA-256 mismatch');
  await store.put(cacheKey(entry.sha256),new Response(bytes as BodyInit,{headers:{'Content-Type':'application/octet-stream'}}));
  return bytes;
}
export async function prepareClient(opts:{
  manifestUrl:string;expectedDigest:string;trustedKeys:TrustedKeys;
  onProgress?:(s:{completed:number;total:number;downloaded:number;phase:string})=>void;
  signal?:AbortSignal;
}):Promise<{manifest:UpdateManifest;wasm:Uint8Array;js:Uint8Array}>{
  const manifestURL=secureUrl(opts.manifestUrl,opts.manifestUrl);
  const res=await fetch(manifestURL,{credentials:'omit',cache:'no-store',signal:opts.signal});
  if(secureUrl(res.url,manifestURL.toString()).origin!==manifestURL.origin)throw new Error('Manifest redirect across origins');
  if(!res.ok)throw new Error('Manifest fetch failed: '+res.status);
  const text=new TextDecoder('utf-8',{fatal:true}).decode(await boundedRead(res,MAX_MANIFEST_BYTES));
  const raw=JSON.parse(text);
  // Signed manifests MUST be serialized canonically to reject duplicate JSON keys.
  // Publisher writes canonical JSON + trailing newline.
  if(text.trimEnd()!==canonicalJson(raw))throw new Error('Noncanonical or duplicate-key release manifest');
  const manifest=await verifyManifest(raw,opts.expectedDigest,opts.trustedKeys);
  const files=[manifest.entry.js,manifest.entry.wasm,...manifest.files];
  if(files.some(file=>file.bytes>MAX_ASSET_BYTES))throw new Error('Asset exceeds update size budget');
  let completed=0,downloaded=0;
  const values=new Map<string,Uint8Array>();
  for(const file of files){
    opts.onProgress?.({completed,total:files.length,downloaded,phase:'Verificando / atualizando arquivos'});
    const bytes=await verifiedBytes(file,opts.manifestUrl,opts.signal);
    values.set(file.sha256,bytes);
    downloaded+=bytes.length;completed++;
    opts.onProgress?.({completed,total:files.length,downloaded,phase:'Arquivos verificados'});
  }
  return {manifest,wasm:values.get(manifest.entry.wasm.sha256)!,js:values.get(manifest.entry.js.sha256)!};
}
// Browser execution remains disabled until original-client entry ABI is hooked into this contract.
export function canLaunchOriginalClient():boolean {return false}
