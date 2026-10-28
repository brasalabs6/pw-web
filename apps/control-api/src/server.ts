import {serve} from '@hono/node-server';
import {app,db} from './app.ts';
const port=Number(process.env.API_PORT||8787);
serve({fetch:app.fetch,port,hostname:process.env.API_BIND||'127.0.0.1'},info=>{
  console.log('PW control API listening at http://'+(process.env.API_BIND||'127.0.0.1')+':'+info.port);
});
process.on('SIGTERM',()=>{void db.end()});
