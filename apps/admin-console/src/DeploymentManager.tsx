import React,{useEffect,useState} from 'react';
import {api} from '@pw/api-sdk';
import {Status} from '@pw/ui';
type Release={id:string;version:string;status:string};
type Realm={id:string;name:string};
export default function DeploymentManager({realms,releases}:{realms:Realm[];releases:Release[]}){
 const [items,setItems]=useState<any[]>([]),[realmId,setRealmId]=useState(''),
 [releaseId,setReleaseId]=useState(''),[gatewayUrl,setGatewayUrl]=useState(''),
 [assetBaseUrl,setAssetBaseUrl]=useState(''),[error,setError]=useState(''),[message,setMessage]=useState('');
 const load=()=>api.request<{deployments:any[]}>('/v1/admin/deployments')
   .then(r=>setItems(r.deployments)).catch(e=>setError(String(e)));
 useEffect(()=>{void load()},[]);
 return <div className="flow" style={{maxWidth:'none'}}>
 <div className="panel"><div className="row"><h2>Deployments / saúde dos servidores</h2><button className="secondary" onClick={load}>Atualizar</button></div>
  <p className="muted">Apenas um probe autenticado que verifica daemons, gs e login→mundo pode declarar gameplay READY. Registrar deployment não inicia o servidor.</p>
  <table><thead><tr><th>Realm</th><th>Servidor</th><th>Estado</th><th>Health atualizado</th></tr></thead><tbody>
   {items.map(d=><tr key={d.id}><td><code>{d.realm_id.slice(0,8)}</code></td>
    <td><code>{d.server_release_id.slice(0,8)}</code></td><td><Status value={d.state}/></td>
    <td>{d.health_checked_at?new Date(d.health_checked_at).toLocaleString('pt-BR'):'Não verificado'}</td></tr>)}
  </tbody></table>
 </div>
 <form className="panel" onSubmit={e=>{e.preventDefault();setError('');setMessage('');
   void api.request('/v1/admin/deployments',{method:'POST',body:JSON.stringify({realmId,serverReleaseId:releaseId,gatewayUrl,assetBaseUrl})})
   .then(()=>{setMessage('Deployment cadastrado. Aguardando health real.');load()}).catch(e=>setError(String(e)))}}>
   <h2>Registrar deployment</h2>
   <label>Realm<select required value={realmId} onChange={e=>setRealmId(e.target.value)}><option value="">Escolher servidor</option>
    {realms.map(r=><option key={r.id} value={r.id}>{r.name}</option>)}</select></label>
   <label>Release de servidor publicada<select required value={releaseId} onChange={e=>setReleaseId(e.target.value)}><option value="">Selecionar release</option>
    {releases.filter(r=>r.status==='published').map(r=><option key={r.id} value={r.id}>{r.version}</option>)}</select></label>
   <label>Gateway WSS<input type="url" required placeholder="wss://gateway.seudominio..." value={gatewayUrl} onChange={e=>setGatewayUrl(e.target.value)}/></label>
   <label>Base HTTPS dos assets<input type="url" required placeholder="https://assets.seudominio..." value={assetBaseUrl} onChange={e=>setAssetBaseUrl(e.target.value)}/></label>
   <button>Registrar pendente</button>
   {error&&<p className="error">{error}</p>}{message&&<p className="success">{message}</p>}
 </form>
 </div>;
}
