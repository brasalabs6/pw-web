import React,{useEffect,useState} from 'react';
import {api} from '@pw/api-sdk';
import {Status} from '@pw/ui';

type Release={id:string;version:string;digest:string;status:string};
export default function ReleaseManager({clientReleases,serverReleases,onMutate,role}:{
 clientReleases:Release[];serverReleases:Release[];onMutate:()=>void;role:string
}){
 const [kind,setKind]=useState<'client'|'server'>('client'),[version,setVersion]=useState(''),
 [digest,setDigest]=useState(''),[artifactUrl,setArtifactUrl]=useState(''),
 [protocol,setProtocol]=useState(''),[data,setData]=useState(''),
 [pairClient,setPairClient]=useState(''),[pairServer,setPairServer]=useState(''),
 [evidence,setEvidence]=useState(''),[pairings,setPairings]=useState<any[]>([]),
 [channels,setChannels]=useState<any[]>([]),[pinRealm,setPinRealm]=useState(''),[pinId,setPinId]=useState(''),
 [error,setError]=useState(''),[message,setMessage]=useState('');
 const reload=()=>Promise.all([
   api.request<{pairings:any[]}>('/v1/admin/pairings').then(r=>setPairings(r.pairings)),
   api.request<{channels:any[]}>('/v1/admin/channels').then(r=>setChannels(r.channels))
 ]).catch(e=>setError(String(e)));
 useEffect(()=>{void reload()},[]);
 const execute=async(action:()=>Promise<unknown>)=>{
   setError('');setMessage('');try{await action();setMessage('Operação registrada com sucesso');onMutate();void reload()}
   catch(e){setError(String(e))}
 };
 const releases=kind==='client'?clientReleases:serverReleases;
 return <div className="flow" style={{maxWidth:'none'}}>
  {error&&<p className="error" role="alert">{error}</p>}{message&&<p className="success">{message}</p>}
  <div className="panel"><h2>Versões publicadas / releases</h2>
   <div className="actions"><button className={kind==='client'?'':'secondary'} onClick={()=>setKind('client')}>Clientes</button>
   <button className={kind==='server'?'':'secondary'} onClick={()=>setKind('server')}>Servidores</button></div>
   <table><thead><tr><th>Versão</th><th>SHA-256</th><th>Estado</th><th>Gestão</th></tr></thead><tbody>
    {releases.map(r=><tr key={r.id}><td>{r.version}</td><td><code>{r.digest.slice(0,24)}…</code></td><td><Status value={r.status}/></td>
    <td><div className="actions">
     {role==='admin'&&r.status==='draft'&&<button className="secondary" onClick={()=>execute(()=>api.request('/v1/admin/releases/'+kind+'/'+r.id+'/status',{method:'POST',body:JSON.stringify({status:'validated',evidenceUrl:evidence})}))}>Validar</button>}
     {role==='admin'&&r.status==='validated'&&<button className="secondary" onClick={()=>execute(()=>api.request('/v1/admin/releases/'+kind+'/'+r.id+'/status',{method:'POST',body:JSON.stringify({status:'published',evidenceUrl:evidence})}))}>Publicar</button>}
     {role==='admin'&&r.status==='published'&&<button className="danger" onClick={()=>execute(()=>api.request('/v1/admin/releases/'+kind+'/'+r.id+'/status',{method:'POST',body:JSON.stringify({status:'revoked',evidenceUrl:evidence})}))}>Revogar</button>}
    </div></td></tr>)}</tbody></table>
   <label>URL HTTPS da evidência de validação (obrigatória para aprovação)<input type="url" value={evidence} onChange={e=>setEvidence(e.target.value)} placeholder="https://github.com/.../actions/runs/..."/></label>
   <p className="muted small">Aprovação humana não substitui validação automática do conteúdo e assinatura no shell.</p>
  </div>
  <form className="panel" onSubmit={e=>{e.preventDefault();void execute(()=>api.request('/v1/admin/releases/'+kind+'s',{
    method:'POST',body:JSON.stringify({version,digest,artifactUrl,protocolProfile:protocol,dataProfile:data})
  }))}}>
    <h2>Registrar release de {kind==='client'?'cliente':'servidor'}</h2>
    <p className="muted">Registro inicia como rascunho. A digest precisa corresponder ao manifesto efetivamente assinado pelo publisher.</p>
    <label>Versão<input required value={version} onChange={e=>setVersion(e.target.value)} placeholder="1.5.3-v145"/></label>
    <label>SHA-256 (64 hex)<input required minLength={64} maxLength={64} pattern="[a-f0-9]{64}" value={digest} onChange={e=>setDigest(e.target.value)}/></label>
    <label>Manifest / artefato HTTPS<input required type="url" value={artifactUrl} onChange={e=>setArtifactUrl(e.target.value)}/></label>
    <div className="grid"><label>Protocol profile<input required value={protocol} onChange={e=>setProtocol(e.target.value)}/></label>
      <label>Data profile<input required value={data} onChange={e=>setData(e.target.value)}/></label></div>
    <button>Cadastrar draft</button>
  </form>
  <form className="panel" onSubmit={e=>{e.preventDefault();void execute(()=>api.request('/v1/admin/pairings',{
    method:'POST',body:JSON.stringify({clientReleaseId:pairClient,serverReleaseId:pairServer,
      protocolProfile:protocol,dataProfile:data,status:role==='admin'?'supported':'untested',evidenceUrl:evidence||undefined})
  }))}}>
    <h2>Matriz de compatibilidade</h2><p className="muted">Somente pares com mesmo protocolo e perfil de dados são elegíveis. Para homologar um par como supported, informe evidência real do teste completo.</p>
    <div className="grid"><label>Client release<select value={pairClient} onChange={e=>setPairClient(e.target.value)} required>
      <option value="">Selecionar cliente</option>{clientReleases.map(r=><option key={r.id} value={r.id}>{r.version} · {r.status}</option>)}</select></label>
      <label>Server release<select value={pairServer} onChange={e=>setPairServer(e.target.value)} required>
      <option value="">Selecionar servidor</option>{serverReleases.map(r=><option key={r.id} value={r.id}>{r.version} · {r.status}</option>)}</select></label></div>
    <button>Registrar pairing</button>
    <table><thead><tr><th>Client ID</th><th>Server ID</th><th>Protocolo</th><th>Estado</th></tr></thead><tbody>
    {pairings.map(p=><tr key={p.id}><td><code>{p.client_release_id.slice(0,8)}</code></td>
    <td><code>{p.server_release_id.slice(0,8)}</code></td><td>{p.protocol_profile}</td><td><Status value={p.status}/></td></tr>)}</tbody></table>
  </form>
  <form className="panel" onSubmit={e=>{e.preventDefault();const ch=channels.find(c=>c.realm_id===pinRealm);
    if(ch)void execute(()=>api.request('/v1/admin/realms/'+pinRealm+'/channels/'+ch.name+'/pin',{
      method:'POST',body:JSON.stringify({pairingId:pinId,expectedRevision:ch.revision})
    }))}}>
   <h2>Versão automática por servidor</h2><p className="muted">O canal stable define o cliente automaticamente. Troca protegida por revisão CAS; sessões em andamento mantêm seu pin.</p>
   <label>Realm/canal<select value={pinRealm} required onChange={e=>setPinRealm(e.target.value)}>
     <option value="">Selecionar realm</option>{channels.map(ch=><option key={ch.realm_id+ch.name} value={ch.realm_id}>{ch.realm_name} / {ch.name} (rev {ch.revision})</option>)}</select></label>
   <label>Pairing supported<select required value={pinId} onChange={e=>setPinId(e.target.value)}>
     <option value="">Selecionar versão homologada</option>
     {pairings.filter(p=>p.status==='supported').map(p=><option key={p.id} value={p.id}>{p.id.slice(0,8)} · {p.protocol_profile}</option>)}
   </select></label>
   <button>Vincular versão ao servidor</button>
  </form>
 </div>;
}
