import {readFileSync,readdirSync} from 'node:fs';
import {createHash} from 'node:crypto';
import {fileURLToPath} from 'node:url';
import {connectDb} from './index.ts';

const db=connectDb();
const client=await db.connect();
try{
  await client.query('SELECT pg_advisory_lock($1)',[64150302]);
  await client.query(`CREATE TABLE IF NOT EXISTS schema_migrations(
    name text PRIMARY KEY,
    digest char(64) NOT NULL,
    applied_at timestamptz NOT NULL DEFAULT now()
  )`);
  const dir=fileURLToPath(new URL('../migrations/',import.meta.url));
  const scripts=readdirSync(dir).filter(n=>/^[0-9]{3}_[a-z0-9_]+\.sql$/.test(n)).sort();
  for(const name of scripts){
    const sql=readFileSync(fileURLToPath(new URL('../migrations/'+name,import.meta.url)),'utf8');
    const digest=createHash('sha256').update(sql).digest('hex');
    const previous=await client.query('SELECT digest FROM schema_migrations WHERE name=$1',[name]);
    if(previous.rowCount){
      if(previous.rows[0].digest!==digest)throw new Error('Migration checksum changed: '+name);
      console.log('SKIP',name);continue;
    }
    // Each migration owns its BEGIN/COMMIT; SQL migrations MUST be safely re-runnable
    // if process crashes after COMMIT but before schema_migrations records the SHA.
    await client.query(sql);
    await client.query('INSERT INTO schema_migrations(name,digest) VALUES($1,$2)',[name,digest]);
    console.log('APPLIED',name);
  }
}finally{
  try{await client.query('SELECT pg_advisory_unlock($1)',[64150302])}finally{client.release();await db.end()}
}
