import {randomBytes, scryptSync, timingSafeEqual, createHash} from 'node:crypto';
export const sha256 = (v:string) => createHash('sha256').update(v).digest('hex');
export const randomToken = () => randomBytes(32).toString('base64url');
export function passwordHash(password:string):string {
  const salt = randomBytes(16).toString('hex');
  const hash = scryptSync(password, salt, 64, {N:16384,r:8,p:1}).toString('hex');
  return 'scrypt$'+salt+'$'+hash;
}
export function verifyPassword(password:string, stored:string):boolean {
  const [algo,salt,hex]=stored.split('$');
  if(algo!=='scrypt'|| !/^[0-9a-f]{32}$/.test(salt??'') || !/^[0-9a-f]{128}$/.test(hex??'')) return false;
  const expected=Buffer.from(hex,'hex');
  return timingSafeEqual(expected,scryptSync(password,salt,expected.length,{N:16384,r:8,p:1}));
}
export function safeOrigin(origin:string|null):boolean {
  if(!origin) return true; // CLI clients without Origin still need cookie/session auth
  const allowed=(process.env.APP_ORIGINS || 'http://localhost:5173,http://localhost:5174,http://localhost:5175')
    .split(',').map(s=>s.trim());
  return allowed.includes(origin);
}
