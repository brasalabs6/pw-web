import {randomBytes, scrypt, timingSafeEqual, createHash} from 'node:crypto';
export const sha256 = (v:string) => createHash('sha256').update(v).digest('hex');
export const randomToken = () => randomBytes(32).toString('base64url');
const derive=(password:string,salt:string,length:number):Promise<Buffer>=>new Promise((resolve,reject)=>{
  scrypt(password,salt,length,{N:16384,r:8,p:1},(error,key)=>error?reject(error):resolve(key));
});
export async function passwordHash(password:string):Promise<string> {
  const salt = randomBytes(16).toString('hex');
  const hash = (await derive(password,salt,64)).toString('hex');
  return 'scrypt$'+salt+'$'+hash;
}
export async function verifyPassword(password:string, stored:string):Promise<boolean> {
  const [algo,salt,hex]=stored.split('$');
  if(algo!=='scrypt'|| !/^[0-9a-f]{32}$/.test(salt??'') || !/^[0-9a-f]{128}$/.test(hex??'')) return false;
  const expected=Buffer.from(hex,'hex');
  return timingSafeEqual(expected,await derive(password,salt,expected.length));
}
export function safeOrigin(origin:string|null):boolean {
  if(!origin) return true; // CLI clients without Origin still need cookie/session auth
  const allowed=(process.env.APP_ORIGINS || 'http://localhost:5173,http://localhost:5174,http://localhost:5175')
    .split(',').map(s=>s.trim());
  return allowed.includes(origin);
}
