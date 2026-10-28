import {catalogRealm,launchReleaseSchema,launchBootstrapSchema,publicUserSchema,
  type CatalogRealm,type LaunchBootstrap,type LaunchRelease} from '@pw/contracts';
export type AuthUser={id:string;email:string;displayName:string;role:string};
export class ApiError extends Error {constructor(public status:number,message:string){super(message)}}
export class PwApi {
  constructor(private base=''){}
  async request<T>(url:string, options:RequestInit={}):Promise<T>{
    if(!url.startsWith('/v1/'))throw new ApiError(400,'Only versioned API paths are allowed');
    // No automatic retry on mutations: an idempotency key belongs to one user action.
    const signal=options.signal?AbortSignal.any([options.signal,AbortSignal.timeout(15000)]):AbortSignal.timeout(15000);
    const r=await fetch(this.base+url,{...options,signal,credentials:'include',headers:{
      'Accept':'application/json',
      ...(options.body?{'Content-Type':'application/json'}:{}),...options.headers}});
    if(r.status===204)return undefined as T;
    const data=await r.json().catch(()=>({error:'Invalid JSON response from API'}));
    if(!r.ok)throw new ApiError(r.status,typeof data.error==='string'?data.error:'Request failed');
    if(data?.error==='Invalid JSON response from API')throw new ApiError(502,'Invalid JSON response from API');
    return data as T;
  }
  async realms(){
    const data=await this.request<{realms:CatalogRealm[]}>('/v1/catalog/realms');
    return {realms:catalogRealm.array().parse(data.realms)};
  }
  async me(){
    const data=await this.request<{user:AuthUser}>('/v1/auth/me');
    return {user:publicUserSchema.parse(data.user)};
  }
  async login(email:string,password:string){
    const data=await this.request<{user:AuthUser}>('/v1/auth/login',{method:'POST',body:JSON.stringify({email,password})});
    return {user:publicUserSchema.parse(data.user)};
  }
  register(email:string,password:string,displayName:string){return this.request<{user:AuthUser}>('/v1/auth/register',{method:'POST',body:JSON.stringify({email,password,displayName})})}
  logout(){return this.request<{ok:boolean}>('/v1/auth/logout',{method:'POST'})}
  async resolveRelease(realmId:string,signal?:AbortSignal){
    const data=await this.request<{release:LaunchRelease}>('/v1/launcher/releases',{
      method:'POST',body:JSON.stringify({realmId,channel:'stable'}),signal});
    return {release:launchReleaseSchema.parse(data.release)};
  }
  async launch(realmId:string,expectedPairingId:string,expectedClientDigest:string){
    const data=await this.request<{bootstrap:LaunchBootstrap}>('/v1/launcher/sessions',{
      method:'POST',body:JSON.stringify({realmId,channel:'stable',expectedPairingId,expectedClientDigest})});
    return {bootstrap:launchBootstrapSchema.parse(data.bootstrap)};
  }
  overview(){return this.request<{realms:any[];clientReleases:any[];serverReleases:any[];jobs:any[]}>('/v1/admin/overview')}
  createRealm(input:{slug:string;name:string;description:string}){return this.request<{realm:any}>('/v1/admin/realms',{method:'POST',body:JSON.stringify(input)})}
  updateRealm(id:string,input:{status?:string;name?:string}){return this.request<{realm:any}>('/v1/admin/realms/'+id,{method:'PATCH',body:JSON.stringify(input)})}
  previews(input:{kind:'client'|'server'|'paired';clientCommit?:string;serverCommit?:string;ttlHours:number}){
    return this.request<{operation:any}>('/v1/admin/previews',{method:'POST',
      headers:{'Idempotency-Key':crypto.randomUUID()},body:JSON.stringify(input)});
  }
  jobs(){return this.request<{jobs:any[]}>('/v1/admin/jobs')}
}
export const api=new PwApi(import.meta.env?.VITE_API_BASE_URL||'');
