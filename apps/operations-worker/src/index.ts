import {connectDb} from '@pw/database';
import {createPreviewInput} from '@pw/contracts';

const sleep=(ms:number)=>new Promise(resolve=>setTimeout(resolve,ms));
const db=connectDb();
const repository=process.env.GITHUB_REPOSITORY||'brasalabs6/pw-web';
if(!/^[a-zA-Z0-9_.-]+\/[a-zA-Z0-9_.-]+$/.test(repository))throw new Error('Invalid repository');
const workflow=process.env.PREVIEW_WORKFLOW||'pw-platform-preview.yml';
if(!/^[a-zA-Z0-9_.-]+\.ya?ml$/.test(workflow))throw new Error('Invalid workflow');
const dispatchBranch=process.env.PREVIEW_WORKFLOW_BRANCH||'feat/multi-realm-platform-shell-mvp';

async function dispatch(job:{id:string;payload:unknown}){
  const token=process.env.GITHUB_TOKEN;
  if(!token)throw new Error('GITHUB_TOKEN missing (dispatch disabled, no fake preview)');
  const p=createPreviewInput.parse(job.payload);
  // Runs a dedicated trusted workflow. Candidate SHAs are data, never shell commands.
  const response=await fetch('https://api.github.com/repos/'+repository+'/actions/workflows/'+workflow+'/dispatches',{
    method:'POST',headers:{'Accept':'application/vnd.github+json','Authorization':'Bearer '+token,
      'X-GitHub-Api-Version':'2022-11-28','User-Agent':'pw-operations-worker','Content-Type':'application/json'},
    body:JSON.stringify({ref:dispatchBranch,inputs:{
      operation_id:job.id,kind:p.kind,client_sha:p.clientCommit||'',server_sha:p.serverCommit||'',
      ttl_hours:String(p.ttlHours)}}),signal:AbortSignal.timeout(15000)});
  if(response.status!==204)throw new Error('GitHub dispatch HTTP '+response.status);
}

let nextMaintenance=0;
async function run(){
  // Expired auth buckets, web sessions and one-use tickets must not grow forever.
  // A web session deletion cascades outstanding game tickets.
  if(Date.now()>=nextMaintenance){
    await db.query("DELETE FROM auth_rate_buckets WHERE bucket_start < now()-interval '1 day'");
    await db.query("DELETE FROM game_tickets WHERE expires_at < now()-interval '2 hours'");
    await db.query("DELETE FROM web_sessions WHERE expires_at < now()-interval '24 hours'");
    nextMaintenance=Date.now()+60*60*1000;
  }
  const token=process.env.GITHUB_TOKEN;
  if(!token){console.warn('GITHUB_TOKEN not configured: preview queue remains waiting');return}
  // Never redispatch after an uncertain worker crash: request manual reconciliation
  // with GitHub run history so an accepted external side-effect is not duplicated.
  await db.query("UPDATE operation_jobs SET state='blocked',reserved_until=NULL,updated_at=now(),result=jsonb_build_object('error','Worker crashed or dispatch timed out; reconcile GitHub before retry') WHERE state='dispatching' AND reserved_until < now()");
  const client=await db.connect();
  let job:{id:string;payload:unknown}|null=null;
  try{
    await client.query('BEGIN');
    const r=await client.query(
      "SELECT id,payload FROM operation_jobs WHERE state='queued' AND kind='preview.create' ORDER BY created_at LIMIT 1 FOR UPDATE SKIP LOCKED");
    if(r.rowCount){
      job=r.rows[0];
      await client.query("UPDATE operation_jobs SET state='dispatching',attempts=attempts+1,reserved_until=now()+interval '90 seconds',updated_at=now() WHERE id=$1",[job!.id]);
    }
    await client.query('COMMIT');
  }catch(e){await client.query('ROLLBACK');throw e}finally{client.release()}
  if(!job)return;
  try{
    await dispatch(job);
    await db.query("UPDATE operation_jobs SET state='dispatched',reserved_until=NULL,result=$2,updated_at=now() WHERE id=$1",
      [job.id,JSON.stringify({message:'GitHub workflow dispatched. Preview readiness NOT proven.'})]);
    console.log('Preview dispatched',job.id);
  }catch(e){
    console.error('Preview dispatch blocked',job.id,e instanceof Error?e.message:'unknown');
    await db.query("UPDATE operation_jobs SET state='blocked',reserved_until=NULL,result=$2,updated_at=now() WHERE id=$1",
      [job.id,JSON.stringify({error:e instanceof Error?e.message:'dispatch error'})]);
  }
}
console.log('PW operations worker started; dispatch does not imply deployed or playable preview.');
process.on('SIGINT',()=>{void db.end().then(()=>process.exit(0))});
while(true){try{await run()}catch(e){console.error('worker error',e)}await sleep(5000)}
