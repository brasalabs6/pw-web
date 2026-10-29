'use strict';

// A clean return is only a completed diagnostic, not proof of a playable game.
function classify(text) {
  let result = 'incomplete';
  for (const line of String(text || '').split('\n')) {
    if (line.startsWith('WORKER ERROR')) return 'failed';
    let message;
    try { message = JSON.parse(line); } catch (_) { continue; }
    if (message.type === 'exception') return 'failed';
    if (message.type === 'return') result = message.result === 0 ? 'returned' : 'failed';
  }
  return result;
}
module.exports = {classify};
