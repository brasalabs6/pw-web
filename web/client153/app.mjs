import createClient from './client153.mjs';
const canvas=document.querySelector('#canvas');
const status=document.querySelector('#status');
const details=document.querySelector('#details');
const report=window.__pw153={status:'LOADING',assets:[],drawCalls:0,frames:0};
let runtime,scene,yaw=.55,pitch=-.15,distance=3.6,drag;
const hash=async bytes=>Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256',bytes)),x=>x.toString(16).padStart(2,'0')).join('');
async function fetchAsset(asset){
 const response=await fetch(asset.url);if(!response.ok)throw Error(`Asset HTTP ${response.status}`);
 const bytes=new Uint8Array(await response.arrayBuffer());
 if(bytes.byteLength!==asset.bytes||await hash(bytes)!==asset.sha256)throw Error(`Asset integrity mismatch: ${asset.path}`);
 report.assets.push({path:asset.path,bytes:bytes.byteLength,sha256:asset.sha256});return bytes;
}
function passBytes(bytes,fn){const ptr=runtime._malloc(bytes.byteLength);if(!ptr)throw Error('WASM allocation failed');try{runtime.HEAPU8.set(bytes,ptr);return fn(ptr,bytes.byteLength);}finally{runtime._free(ptr);}}
function requireResult(value){if(value<=0)throw Error(runtime.UTF8ToString(runtime._client_error()));return value;}
async function initialize(){
 report.status='LOADING';report.assets=[];report.drawCalls=0;report.frames=0;
 requireResult(runtime._client_init());
 const bytes=await fetchAsset(scene.model);
 report.meshes=passBytes(bytes,(ptr,size)=>requireResult(runtime._client_load(ptr,size)));
 const decoder=new TextDecoder('gbk',{fatal:true});
 report.mipLevels=[];
 for(let i=0;i<runtime._client_mesh_count();i++){
  const ptr=runtime._client_texture_name(i),size=runtime._client_texture_name_size(i);
  const name=decoder.decode(runtime.HEAPU8.slice(ptr,ptr+size)).replaceAll('\\','/').split('/').at(-1).toLowerCase();
  const asset=scene.textures[name];if(!asset)throw Error(`Missing original texture dependency: ${name}`);
  const texture=await fetchAsset(asset);report.mipLevels.push(passBytes(texture,(p,n)=>requireResult(runtime._client_texture(i,p,n))));
 }
 const gl=canvas.getContext('webgl2');const info=gl.getExtension('WEBGL_debug_renderer_info');
 report.renderer=gl.getParameter(info?info.UNMASKED_RENDERER_WEBGL:gl.RENDERER);report.webgl=gl.getParameter(gl.VERSION);
 report.status='READY';status.textContent='Modelo original carregado em WASM/WebGL2.';details.textContent=JSON.stringify(report,null,2);
}
function fail(error){if(runtime)runtime._client_dispose();report.status='FAIL';report.error=String(error);status.textContent=report.error;details.textContent=JSON.stringify(report,null,2);console.error(error);}
function frame(){
 if(report.status==='READY'){
  const ratio=Math.min(devicePixelRatio,2);const width=Math.max(1,Math.floor(canvas.clientWidth*ratio)),height=Math.max(1,Math.floor(canvas.clientHeight*ratio));
  if(canvas.width!==width||canvas.height!==height){canvas.width=width;canvas.height=height;}
  try{report.drawCalls=requireResult(runtime._client_draw(yaw,pitch,distance,width,height));report.frames++;if(report.frames===1||report.frames%120===0)details.textContent=JSON.stringify(report,null,2);}catch(error){fail(error);}
 }
 requestAnimationFrame(frame);
}
canvas.addEventListener('pointerdown',event=>{drag={x:event.clientX,y:event.clientY};canvas.setPointerCapture(event.pointerId);});
canvas.addEventListener('pointermove',event=>{if(!drag)return;yaw+=(event.clientX-drag.x)*.01;pitch=Math.max(-1.4,Math.min(1.4,pitch+(event.clientY-drag.y)*.01));drag={x:event.clientX,y:event.clientY};});
canvas.addEventListener('pointerup',()=>{drag=null;});canvas.addEventListener('pointercancel',()=>{drag=null;});
canvas.addEventListener('wheel',event=>{event.preventDefault();distance=Math.max(2.1,Math.min(12,distance+event.deltaY*.002));},{passive:false});
canvas.addEventListener('webglcontextlost',()=>{report.status='LOST';status.textContent='Contexto gráfico perdido.';});
canvas.addEventListener('webglcontextrestored',()=>{initialize().catch(fail);});
try{
 const response=await fetch('scene.json');if(!response.ok)throw Error('Original asset manifest missing');scene=await response.json();
 runtime=await createClient({canvas});await initialize();requestAnimationFrame(frame);
}catch(error){fail(error);}
