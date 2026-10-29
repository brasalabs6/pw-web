'use strict';
// Exercise the real browser probe reader across the signed 32-bit boundary.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

const nodes = new Map();
const requests = [];
const size = 2 ** 31 + 16;
const status = {textContent: ''};
class XMLHttpRequest {
  open(_method, url) { this.url = url; }
  setRequestHeader(_name, value) { this.range = value; }
  overrideMimeType() {}
  send() {
    if (this.url === '/manifest') {
      this.status = 200;
      this.responseText = JSON.stringify({packages: [{name: 'large.pck', size, url: '/large'}]});
      return;
    }
    requests.push(this.range);
    const match = /^bytes=(\d+)-(\d+)$/.exec(this.range);
    if (!match) { this.status = 416; return; }
    const start = Number(match[1]), end = Number(match[2]);
    if (start > end || end >= size) { this.status = 416; return; }
    this.status = 206;
    this.responseText = '*'.repeat(end - start + 1);
  }
}
const FS = {
  createDataFile(_parent, name) { nodes.set('/' + name, {}); },
  lookupPath(name) { return {node: nodes.get(name)}; },
  writeFile() {},
  ErrnoError: class extends Error {},
};
const context = {XMLHttpRequest, FS, Uint8Array, Map, JSON, Number, Math, console,
  specialHTMLTargets: {}, pwCanvas: {}, pwStatus: status};
context.globalThis = context;
const script = fs.readFileSync(path.join(__dirname, '../../scripts/client153-entry-browser-pre.js'), 'utf8');
vm.runInNewContext(script, context);
context.Module._pw153_browser_probe = () => {
  const buffer = new Uint8Array(8);
  assert.equal(nodes.get('/large.pck').stream_ops.read({}, buffer, 0, 8, 2 ** 31), 8);
  assert.deepEqual([...buffer], Array(8).fill(42));
  return 0;
};
context.Module.onRuntimeInitialized();
assert.ok(!status.textContent.includes('EXCEPTION:'), status.textContent);
assert.deepEqual(requests, [`bytes=${2 ** 31}-${size - 1}`]);
console.log('PASS: range reads at 2 GiB');
