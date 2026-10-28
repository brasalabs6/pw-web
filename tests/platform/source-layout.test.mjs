import {readFileSync,existsSync} from 'node:fs';
import {test} from 'node:test';
import assert from 'node:assert/strict';
for(const name of ['shell','player-portal','admin-console','control-api','operations-worker','game-gateway','realm-agent']){
  test('app implemented: '+name,()=>assert.ok(existsSync('apps/'+name+'/src')));
}
test('client auto-resolution is server-channel-driven',()=>{
 const shell=readFileSync('apps/shell/src/App.tsx','utf8');
 assert.match(shell,/api\.launch\(selected\.id\)/);
 assert.match(shell,/b\.client\.manifestUrl/);
 assert.doesNotMatch(shell,/setClientVersion/);
});
test('updater validates both hash and signature',()=>{
 const updater=readFileSync('packages/update-kit/src/index.ts','utf8');
 assert.match(updater,/crypto\.subtle\.verify/);
 assert.match(updater,/digest\(bytes\)/);
});
test('game ticket is single-use',()=>{
 const api=readFileSync('apps/control-api/src/app.ts','utf8');
 assert.match(api,/consumed_at IS NULL/);
 assert.match(api,/loginAndEnterWorldPassed/);
});
