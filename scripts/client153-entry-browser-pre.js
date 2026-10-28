// The browser entry probe pre-js (worker context): maps the client's real
// packages as Range-request-backed read-only MEMFS streams with a 1 MiB
// aligned read-ahead block cache (the same block design the analyzed
// production port uses), provisions the windowed SystemSettings.ini the
// original expects, then drives the original entry through
// pw153_browser_probe and reports over the worker postMessage channel.
//
// Read-ahead: each stream read that misses the cache fetches a full 1 MiB
// aligned block instead of just the requested bytes, reducing the request
// count by 10-40x (the measured bottleneck is per-request overhead, not
// bandwidth). The LRU cache keeps up to PW153_CACHE_BLOCKS blocks per
// package. The sync XHR design is preserved (the original client performs
// synchronous reads; the worker accommodates them).
//
// Design evolved from the Codex m-link wave draft (2026-10-28) and the
// operator's measured boot analysis; the block cache follows the production
// port's 1 MiB-aligned block pattern.
var Module = {
  canvas: globalThis.pwCanvas,
  print: text => postMessage({type: 'log', text: String(text)}),
  printErr: text => postMessage({type: 'error', text: String(text)}),
  onRuntimeInitialized() {
    // 1 MiB aligned read-ahead blocks (matches the production port's block
    // size; the measured request overhead is ~40ms so fewer, larger reads
    // win). The LRU keeps a bounded working set per package.
    const BLOCK_SHIFT = 20; // 1 MiB
    const BLOCK_SIZE = 1 << BLOCK_SHIFT;
    const MAX_CACHED_BLOCKS = 64; // 64 MiB per package

    function request(url, range) {
      const xhr = new XMLHttpRequest();
      xhr.open('GET', url, false);
      if (range) { xhr.responseType = 'arraybuffer'; xhr.setRequestHeader('Range', range); }
      xhr.send();
      if (xhr.status !== (range ? 206 : 200)) throw Error('HTTP ' + xhr.status + ' ' + url);
      return range ? new Uint8Array(xhr.response) : xhr.responseText;
    }

    // Per-package block cache: Map<blockIndex, Uint8Array> with LRU
    // eviction. The insertion order doubles as the LRU order (oldest first).
    function createBlockCache(url, size) {
      const blocks = new Map();
      let requestCount = 0;
      let bytesFetched = 0;
      function fetchBlock(index) {
        const start = index << BLOCK_SHIFT;
        const end = Math.min(start + BLOCK_SIZE, size) - 1;
        if (start >= size) return null;
        const data = request(url, `bytes=${start}-${end}`);
        requestCount++;
        bytesFetched += data.length;
        return data;
      }
      function getBlock(index) {
        if (blocks.has(index)) {
          // LRU refresh: delete and re-insert to move to the end.
          const data = blocks.get(index);
          blocks.delete(index);
          blocks.set(index, data);
          return data;
        }
        const data = fetchBlock(index);
        if (!data) return null;
        blocks.set(index, data);
        if (blocks.size > MAX_CACHED_BLOCKS) {
          // Evict the oldest entry (first key in insertion order).
          const oldest = blocks.keys().next().value;
          blocks.delete(oldest);
        }
        return data;
      }
      return {
        read(position, length) {
          const result = new Uint8Array(length);
          let written = 0;
          while (written < length) {
            const index = (position + written) >>> BLOCK_SHIFT;
            const block = getBlock(index);
            if (!block) break;
            const blockOffset = (position + written) - (index << BLOCK_SHIFT);
            const available = Math.min(block.length - blockOffset, length - written);
            if (available <= 0) break;
            result.set(block.subarray(blockOffset, blockOffset + available), written);
            written += available;
          }
          return result.subarray(0, written);
        },
        stats() { return { requestCount, bytesFetched, cachedBlocks: blocks.size }; }
      };
    }

    try {
      const manifest = JSON.parse(request('/manifest'));
      const caches = [];
      for (const item of manifest.packages) {
        FS.createDataFile('/', item.name, null, true, false);
        const node = FS.lookupPath('/' + item.name).node;
        node.usedBytes = item.size;
        const cache = createBlockCache(item.url, item.size);
        caches.push(cache);
        node.stream_ops = {
          open(stream) { if (stream.flags & (3 | 64 | 512 | 1024)) throw new FS.ErrnoError(69); },
          close() {},
          read(stream, buffer, offset, length, position) {
            const count = Math.min(length, item.size - position);
            if (count <= 0) return 0;
            const data = cache.read(position, count);
            if (data.length !== count) throw Error('Short read: got ' + data.length + ' of ' + count);
            buffer.set(data, offset);
            return count;
          },
          llseek(stream, offset, whence) {
            const pos = offset + (whence === 1 ? stream.position : whence === 2 ? item.size : 0);
            if (whence < 0 || whence > 2 || pos < 0 || !Number.isSafeInteger(pos)) throw new FS.ErrnoError(28);
            return pos;
          },
          write() { throw new FS.ErrnoError(69); }
        };
      }
      FS.writeFile('/userdata' + String.fromCharCode(92) + 'SystemSettings.ini', '[Video]\nFullScreen=0\n');
      // In a worker there is no document, so emscripten's default
      // specialHTMLTargets has no '#canvas' entry; register the offscreen
      // canvas the page transferred so emscripten_webgl_create_context
      // resolves it (the pre-js runs inside the module closure).
      specialHTMLTargets['#canvas'] = globalThis.pwCanvas;
      postMessage({type: 'entry'});
      const result = Module._pw153_browser_probe();
      // Report the block-cache statistics so the speedup is measured, not felt.
      let totalRequests = 0, totalBytes = 0;
      for (const c of caches) {
        const s = c.stats();
        totalRequests += s.requestCount;
        totalBytes += s.bytesFetched;
      }
      postMessage({type: 'return', result, blockCache: { requests: totalRequests, bytesFetched: totalBytes }});
      // Dump the client's own logs (the CECConfigs init chain) like the
      // Node probe does, so the boot diagnosis is identical.
      for (const name of ['AF.log', 'EC.log']) {
        try {
          const text = FS.readFile('/logs/' + name, {encoding: 'utf8'});
          postMessage({type: 'vfslog', name, text: String(text)});
        } catch (_) { /* absent logs are not an error */ }
      }
    } catch (error) { postMessage({type: 'exception', text: String(error.stack || error)}); }
  }
};
