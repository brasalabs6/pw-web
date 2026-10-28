// Safe, opt-in seed: an OFFLINE realm without players, credentials or GNET readiness.
import {connectDb} from './index.ts';
const db=connectDb();
try{
 const q=await db.query("INSERT INTO realms(slug,name,description,status,default_channel) VALUES('pwbr-153-lab','PW Brasil 1.5.3 — Laboratório','Servidor experimental; indisponível até homologação do cliente WASM/GNET.','offline','stable') ON CONFLICT(slug) DO UPDATE SET status='offline' RETURNING id");
 await db.query("INSERT INTO realm_channels(realm_id,name) VALUES($1,'stable') ON CONFLICT DO NOTHING",[q.rows[0].id]);
 console.log('Seeded offline realm pwbr-153-lab:',q.rows[0].id);
}finally{await db.end()}
