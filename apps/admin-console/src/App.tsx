import React,{useEffect,useState} from 'react';
import {api,type AuthUser} from '@pw/api-sdk';
import {Layout,AuthPanel,Status} from '@pw/ui';
import ReleaseManager from './ReleaseManager.tsx';
import DeploymentManager from './DeploymentManager.tsx';
type Overview=Awaited<ReturnType<typeof api.overview>>;
export default function App(){
  const [user,setUser]=useState<AuthUser|null>(null),[data,setData]=useState<Overview|null>(null),
    [tab,setTab]=useState<'realms'|'releases'|'previews'|'deployments'>('realms'),[error,setError]=useState(''),[info,setInfo]=useState(''),
    [slug,setSlug]=useState(''),[name,setName]=useState(''),[description,setDescription]=useState(''),
    [kind,setKind]=useState<'client'|'server'|'paired'>('client'),[clientCommit,setClientCommit]=useState(''),
    [serverCommit,setServerCommit]=useState(''),[ttl,setTtl]=useState(24);
  const refresh=()=>api.overview().then(setData).catch(e=>setError(String(e)));
  useEffect(()=>{api.me().then(r=>{setUser(r.user);if(r.user.role!=='player')refresh()}).catch(()=>{})},[]);
  const send=async(fn:()=>Promise<unknown>)=>{setInfo('');setError('');try{await fn();setInfo('Operação registrada. Verifique o estado atualizado.');refresh()}catch(e){setError(String(e))}};
  return <Layout title="Controle PW Web" subtitle="Administre realms, versões, servidores e previews. Operações são registradas e auditadas."
  actions={<div className="actions"><a className="muted small" href="http://localhost:5173">Launcher</a>
     {user&&<button className="secondary" onClick={async()=>{await api.logout();setUser(null);setData(null)}}>Sair</button>}</div>}>
  {!user?<AuthPanel onLogin={u=>{setUser(u);if(u.role!=='player')refresh()}}/>:user.role==='player'?
    <div className="panel error">Acesso restrito a operadores. Uma conta nova não recebe papel administrativo automaticamente.</div>:
    <div className="flow" style={{maxWidth:'none'}}>
      <div className="actions"><button className={tab==='realms'?'':'secondary'} onClick={()=>setTab('realms')}>Servidores</button>
        <button className={tab==='releases'?'':'secondary'} onClick={()=>setTab('releases')}>Versões</button>
        <button className={tab==='previews'?'':'secondary'} onClick={()=>setTab('previews')}>Previews / Jobs</button>
        <button className={tab==='deployments'?'':'secondary'} onClick={()=>setTab('deployments')}>Deployments</button>
        <button className="secondary" onClick={refresh}>Atualizar</button></div>
      {error&&<p role="alert" className="error">{error}</p>}
      {info&&<p role="status" className="success">{info}</p>}
      {tab==='realms'&&<><div className="panel"><h2>Servidores cadastrados</h2><table><thead><tr><th>Realm</th><th>Estado</th><th>Jogadores</th><th>Ação</th></tr></thead>
        <tbody>{data?.realms.map(r=><tr key={r.id}><td>{r.name}<p className="muted small">{r.slug}</p></td><td><Status value={r.status}/></td>
        <td>{r.population??'—'}</td><td><button className="secondary" onClick={()=>send(()=>api.updateRealm(r.id,{status:'maintenance'}))}>Manutenção</button></td></tr>)}</tbody></table></div>
        <form className="panel" onSubmit={e=>{e.preventDefault();void send(()=>api.createRealm({slug,name,description}))}}>
          <h2>Criar servidor</h2><p className="muted">O cadastro inicia offline; provisionamento e readiness de jogo são operações independentes.</p>
          <div className="grid"><label>Slug<input required pattern="[a-z0-9][a-z0-9-]{2,47}" value={slug} onChange={e=>setSlug(e.target.value)}/></label>
          <label>Nome<input required value={name} onChange={e=>setName(e.target.value)}/></label></div>
          <label>Descrição<textarea value={description} maxLength={400} onChange={e=>setDescription(e.target.value)}/></label><button>Criar realm</button></form></>}
      {tab==='releases'&&data&&<ReleaseManager clientReleases={data.clientReleases} serverReleases={data.serverReleases} onMutate={refresh} role={user.role}/>}
      {tab==='deployments'&&data&&<DeploymentManager realms={data.realms} releases={data.serverReleases}/>}
      {tab==='previews'&&<><form className="panel" onSubmit={e=>{e.preventDefault();void send(()=>api.previews({kind,clientCommit:clientCommit||undefined,serverCommit:serverCommit||undefined,ttlHours:ttl}))}}>
       <h2>Solicitar preview isolado</h2><p className="muted">Informe o commit completo de cada candidato. O worker só despacha o workflow configurado e não anuncia READY sem teste real.</p>
       <label>Tipo<select value={kind} onChange={e=>setKind(e.target.value as typeof kind)}><option value="client">Cliente</option><option value="server">Servidor</option><option value="paired">Pareado</option></select></label>
       {kind!=='server'&&<label>Commit do cliente<input required pattern="[a-f0-9]{40}" value={clientCommit} onChange={e=>setClientCommit(e.target.value)} placeholder="SHA completo"/></label>}
       {kind!=='client'&&<label>Commit do servidor<input required pattern="[a-f0-9]{40}" value={serverCommit} onChange={e=>setServerCommit(e.target.value)} placeholder="SHA completo"/></label>}
       <label>TTL em horas<input type="number" min={1} max={72} value={ttl} onChange={e=>setTtl(Number(e.target.value))}/></label>
       <button>Registrar e despachar preview</button></form>
       <div className="panel"><h2>Fila de operações</h2><table><thead><tr><th>Tipo</th><th>Estado</th><th>Data</th><th>ID</th></tr></thead><tbody>
       {data?.jobs.map(j=><tr key={j.id}><td>{j.kind}</td><td><Status value={j.state}/></td><td>{new Date(j.created_at).toLocaleString('pt-BR')}</td><td><code>{j.id.slice(0,8)}</code></td></tr>)}
       </tbody></table></div></>}
    </div>}
 </Layout>
}
