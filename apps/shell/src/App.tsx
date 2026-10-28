import React,{useEffect,useState} from 'react';
import {api,type AuthUser} from '@pw/api-sdk';
import type {CatalogRealm,LaunchBootstrap} from '@pw/contracts';
import {Layout,Status,AuthPanel} from '@pw/ui';
import {prepareClient} from '@pw/update-kit';

type Stage='choose'|'login'|'checking'|'updating'|'ready'|'error';
function signingKeys():Record<string,string>{
  try{return JSON.parse(import.meta.env.VITE_RELEASE_KEYS_JSON||'{}')}catch{return {}}
}
export default function App(){
  const [realms,setRealms]=useState<CatalogRealm[]>([]),[selected,setSelected]=useState<CatalogRealm|null>(null),
    [user,setUser]=useState<AuthUser|null>(null),[stage,setStage]=useState<Stage>('choose'),
    [error,setError]=useState(''),[progress,setProgress]=useState({completed:0,total:0,downloaded:0,phase:''}),
    [bootstrap,setBootstrap]=useState<LaunchBootstrap|null>(null);
  const refresh=()=>api.realms().then(r=>setRealms(r.realms)).catch(e=>setError(String(e)));
  useEffect(()=>{refresh();api.me().then(r=>setUser(r.user)).catch(()=>{})},[]);
  const choose=(r:CatalogRealm)=>{setSelected(r);setStage(user?'choose':'login');setError('')};
  const launch=async()=>{
    if(!selected)return;setError('');setStage('checking');
    try{
      const {release:r}=await api.resolveRelease(selected.id);
      setStage('updating');
      // Server release determines the client; no client version selection is shown to players.
      const prepared=await prepareClient({
        manifestUrl:r.client.manifestUrl,expectedDigest:r.client.releaseDigest,
        trustedKeys:signingKeys(),onProgress:setProgress
      });
      // Binary verified and cached; launch requires the still-under-development Angelica entry ABI.
      if(prepared.wasm.byteLength===0)throw new Error('Runtime WASM inválido');
      // The runtime is not wired yet: do not waste a 90-second one-use ticket.
      // When the game adapter exists, api.launch(selected.id,r.pairingId,r.client.releaseDigest)
      // must be called immediately before opening the WSS connection.
      setBootstrap({...r,gateway:{url:'',ticket:'',ticketExpiresAt:''}});
      setStage('ready');
    }catch(e){setError(e instanceof Error?e.message:String(e));setStage('error')}
  };
  return <Layout title="Escolha seu mundo." subtitle="Selecione um servidor, entre com sua conta e o PW Web selecionará e atualizará o cliente compatível automaticamente."
    actions={<div className="actions">{user&&<span className="muted small">{user.displayName}</span>}
      <a href="http://localhost:5174" className="muted small">Minha conta</a>
      {user&&<button className="secondary" onClick={async()=>{await api.logout();setUser(null);setStage('login')}}>Sair</button>}</div>}>
    <p className="kicker">01 / Servidores disponíveis <button className="text-button" onClick={refresh}>Atualizar lista</button></p>
    <section className="grid" aria-label="Lista de servidores">
      {!realms.length&&<div className="panel"><p>Nenhum servidor cadastrado. O painel administrativo permite criar realms.</p></div>}
      {realms.map(r=><article className={'realm-card '+(selected?.id===r.id?'selected':'')} key={r.id}>
        <div className="row"><span className="kicker">{r.slug}</span><Status value={r.status}/></div>
        <h2>{r.name}</h2><p>{r.description||'Servidor Perfect World'}</p>
        <div className="row"><span className="muted small">{r.population===null?'Jogadores: indisponível':r.population+' online'}</span>
          <button className={selected?.id===r.id?'secondary':''} onClick={()=>choose(r)}>{selected?.id===r.id?'Selecionado':'Selecionar servidor'}</button></div>
      </article>)}
    </section>
    {selected&&<section className="flow" style={{marginTop:28}}>
      <div className="panel"><div className="row"><div><span className="kicker">02 / Servidor selecionado</span><h2>{selected.name}</h2>
        <p className="muted">Cliente e atualizações selecionados automaticamente pelo servidor.</p></div><Status value={selected.status}/></div></div>
      {stage==='login'&&!user&&<AuthPanel onLogin={u=>{setUser(u);setStage('choose')}}/>}
      {(stage==='choose'||stage==='error')&&user&&<div className="panel"><h2>Pronto para verificar o cliente</h2>
        <p className="muted">O launcher encontrará a versão homologada, verificará a assinatura e atualizará somente os arquivos necessários.</p>
        <button onClick={launch} disabled={selected.status!=='online'}>Verificar atualizações e jogar</button>
        {selected.status!=='online'&&<p className="muted small">Servidor indisponível: não é permitido iniciar um jogo sem backend pronto.</p>}</div>}
      {(stage==='checking'||stage==='updating')&&<div className="panel" aria-live="polite"><h2>{stage==='checking'?'Verificando compatibilidade…':'Atualizando cliente…'}</h2>
        <p>{progress.phase}</p><div className="progress"><div style={{width:progress.total?100*progress.completed/progress.total+'%':'0%'}}/></div>
        <p className="muted small">{progress.completed} / {progress.total} arquivos · {Math.round(progress.downloaded/1024/1024*10)/10} MiB verificados</p></div>}
      {stage==='error'&&error&&<p role="alert" className="error">{error}</p>}
      {stage==='ready'&&bootstrap&&<div className="panel"><h2>Arquivos atualizados e verificados</h2>
        <p className="success">Release {bootstrap.client.version} preparada e armazenada no cache local.</p>
        <p className="muted">A inicialização do gameplay ainda depende da integração do entry ABI do cliente Angelica portado no PR #23. Não iniciamos um runtime incompleto nem simulamos conexão ao jogo.</p>
        <button disabled>Iniciar o jogo — integração pendente</button></div>}
    </section>}
    {error&&stage!=='error'&&<p className="error" role="alert">{error}</p>}
  </Layout>;
}
