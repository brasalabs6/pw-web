import React,{useEffect,useState} from 'react';
import {api,type AuthUser} from '@pw/api-sdk';
import {Layout,AuthPanel} from '@pw/ui';
export default function App(){
 const [user,setUser]=useState<AuthUser|null>(null),[error,setError]=useState('');
 useEffect(()=>{api.me().then(r=>setUser(r.user)).catch(()=>{})},[]);
 return <Layout title="Minha conta PW Web" subtitle="Gerencie sua identidade web e acesse os servidores disponíveis."
   actions={<a href={import.meta.env.VITE_SHELL_URL||'http://localhost:5173'} className="muted">Voltar ao launcher</a>}>
   {!user?<AuthPanel onLogin={setUser}/>:<div className="panel" style={{maxWidth:650}}>
     <span className="kicker">Conta autenticada</span><h2>{user.displayName}</h2>
     <p><strong>E-mail:</strong> {user.email}</p>
     <p className="muted">Personagens, saldo de gold, recuperação e gestão de contas GNET ainda exigem a ponte de identidade com o servidor original; nenhuma operação em WDB é simulada.</p>
     <button className="secondary" onClick={async()=>{try{await api.logout();setUser(null)}catch(e){setError(String(e))}}}>Sair</button>
     {error&&<p role="alert" className="error">{error}</p>}
   </div>}
 </Layout>
}
