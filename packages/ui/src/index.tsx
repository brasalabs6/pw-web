import React,{useState} from 'react';
import {api,ApiError,type AuthUser} from '@pw/api-sdk';
export function Layout({title,subtitle,children,actions}:{title:string;subtitle?:string;children:React.ReactNode;actions?:React.ReactNode}){
  return <div className="layout"><header className="topbar"><div className="brand"><span className="brandmark">PW<span> / WEB</span></span><span className="toplabel">PLATAFORMA MULTIREALM</span></div>{actions}</header><main><div className="pagehead"><h1>{title}</h1>{subtitle&&<p>{subtitle}</p>}</div>{children}</main><footer>PW Web — ambiente de desenvolvimento. Disponibilidade e compatibilidade dependem de provas reais.</footer></div>;
}
export function Status({value}:{value:string}){return <span className={'status status-'+value}>{value}</span>}
export function AuthPanel({onLogin}:{onLogin:(u:AuthUser)=>void}){
  const [mode,setMode]=useState<'login'|'register'>('login'),[email,setEmail]=useState(''),
    [password,setPassword]=useState(''),[displayName,setDisplayName]=useState(''),
    [error,setError]=useState(''),[busy,setBusy]=useState(false);
  return <section className="panel auth-panel"><h2>{mode==='login'?'Entrar na minha conta':'Criar minha conta'}</h2>
  <p className="muted">Esta é a identidade web. A integração com o login GNET do jogo ainda depende do adaptador nativo.</p>
  <form onSubmit={async e=>{e.preventDefault();setBusy(true);setError('');try{
    if(mode==='register'){await api.register(email,password,displayName)}
    const r=await api.login(email,password);onLogin(r.user);
  }catch(e){setError(e instanceof ApiError?e.message:'Não foi possível autenticar')}finally{setBusy(false)}}}>
    {mode==='register'&&<label>Nome<input required minLength={2} maxLength={60} value={displayName} onChange={e=>setDisplayName(e.target.value)}/></label>}
    <label>E-mail<input type="email" required autoComplete="username" value={email} onChange={e=>setEmail(e.target.value)}/></label>
    <label>Senha<input type="password" required minLength={mode==='register'?12:1} autoComplete={mode==='register'?'new-password':'current-password'} value={password} onChange={e=>setPassword(e.target.value)}/></label>
    {error&&<p role="alert" className="error">{error}</p>}
    <button disabled={busy} type="submit">{busy?'Processando…':mode==='login'?'Entrar':'Criar conta e entrar'}</button>
    <button className="text-button" type="button" onClick={()=>setMode(mode==='login'?'register':'login')}>{mode==='login'?'Criar uma conta':'Já tenho conta'}</button>
  </form></section>;
}
