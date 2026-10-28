import type {CatalogRealm,LaunchBootstrap} from '@pw/contracts';
export type AuthUser={id:string;email:string;displayName:string;role:string};
export class ApiError extends Error {constructor(public status:number,message:string){super(message)}}
export class PwApi {
  constructor(private base=''){}
  async request<T>(url:string, options:RequestInit={}):Promise<T>{
    const r=await fetch(this.base+url,{...options,credentials:'include',headers:{
      ...(options.body?{'Content-Type':'application/json'}:{}),...options.headers}});
    const data=await r.json().catch(()=>({error:'Invalid response from API'}));
    if(!r.ok)throw new ApiError(r.status,String(data.error||'Request failed'));
    return data as T;
  }
  realms(){return this.request<{realms:CatalogRealm[]}>('/v1/catalog/realms')}
  me(){return this.request<{user:AuthUser}>('/v1/auth/me')}
  login(email:string,password:string){return this.request<{user:AuthUser}>('/v1/auth/login',{method:'POST',body:JSON.stringify({email,password})})}
  register(email:string,password:string,displayName:string){return this.request<{user:AuthUser}>('/v1/auth/register',{method:'POST',body:JSON.stringify({email,password,displayName})})}
  logout(){return this.request<{ok:boolean}>('/v1/auth/logout',{method:'POST'})}
  launch(realmId:string){return this.request<{bootstrap:LaunchBootstrap}>('/v1/launcher/sessions',{method:'POST',body:JSON.stringify({realmId,channel:'stable'})})}
  overview(){return this.request<{realms:any[];clientReleases:any[];serverReleases:any[];jobs:any[]}>('/v1/admin/overview')}
  createRealm(input:{slug:string;name:string;description:string}){return this.request<{realm:any}>('/v1/admin/realms',{method:'POST',body:JSON.stringify(input)})}
  updateRealm(id:string,input:{status?:string;name?:string}){return this.request<{realm:any}>('/v1/admin/realms/'+id,{method:'PATCH',body:JSON.stringify(input)})}
  previews(input:{kind:'client'|'server'|'paired';clientCommit?:string;serverCommit?:string;ttlHours:number}){
    return this.request<{operation:any}>('/v1/admin/previews',{method:'POST',
      headers:{'Idempotency-Key':crypto.randomUUID()},body:JSON.stringify(input)});
  }
  jobs(){return this.request<{jobs:any[]}>('/v1/admin/jobs')}
}
export const api=new PwApi();
