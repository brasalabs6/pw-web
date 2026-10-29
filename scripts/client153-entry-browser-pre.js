// The browser entry probe pre-js (MAIN THREAD context): maps the client's
// real packages as Range-request-backed read-only MEMFS streams, provisions
// the windowed SystemSettings.ini, then drives the original entry through
// pw153_browser_probe. All output goes to console.log (visible in the
// Chrome DevTools console) and to the status element on the page.
//
// This version runs on the MAIN THREAD (no worker, no offscreen canvas)
// so the WebGL2 context renders directly on the visible canvas element.
var Module = {
  canvas: globalThis.pwCanvas,
  print: text => console.log('[pw153]', text),
  printErr: text => console.error('[pw153]', text),
  onRuntimeInitialized() {
    const status = globalThis.pwStatus;
    function log(msg) {
      console.log('[pw153]', msg);
      if (status) status.textContent += '\n' + msg;
    }
    function request(url, range) {
      const xhr = new XMLHttpRequest();
      xhr.open('GET', url, false);
      if (range) xhr.setRequestHeader('Range', range);
      // On the main thread, sync XHR cannot use responseType='arraybuffer'.
      // The x-user-defined charset preserves raw bytes in responseText.
      xhr.overrideMimeType('text/plain; charset=x-user-defined');
      xhr.send();
      if (xhr.status !== (range ? 206 : 200)) throw Error('HTTP ' + xhr.status + ' ' + url);
      if (range) {
        const text = xhr.responseText;
        const data = new Uint8Array(text.length);
        for (let i = 0; i < text.length; i++) data[i] = text.charCodeAt(i) & 0xff;
        return data;
      }
      return xhr.responseText;
    }
    try {
      const manifest = JSON.parse(request('/manifest'));
      for (const item of manifest.packages) {
        FS.createDataFile('/', item.name, null, true, false);
        const node = FS.lookupPath('/' + item.name).node;
        node.usedBytes = item.size;
        const BLOCK_SHIFT = 20, BLOCK_SIZE = 1 << BLOCK_SHIFT, MAX_CACHED = 64;
        const blocks = new Map();
        function fetchBlock(index) {
          const start = index * BLOCK_SIZE;
          const end = Math.min(start + BLOCK_SIZE, item.size) - 1;
          if (start >= item.size) return null;
          return request(item.url, `bytes=${start}-${end}`);
        }
        function getBlock(index) {
          if (blocks.has(index)) {
            const d = blocks.get(index); blocks.delete(index); blocks.set(index, d); return d;
          }
          const d = fetchBlock(index);
          if (!d) return null;
          blocks.set(index, d);
          if (blocks.size > MAX_CACHED) { blocks.delete(blocks.keys().next().value); }
          return d;
        }
        node.stream_ops = {
          open(stream) { if (stream.flags & (3 | 64 | 512 | 1024)) throw new FS.ErrnoError(69); },
          close() {},
          read(stream, buffer, offset, length, position) {
            const count = Math.min(length, item.size - position);
            if (count <= 0) return 0;
            const result = new Uint8Array(count);
            let written = 0;
            while (written < count) {
              const index = Math.floor((position + written) / BLOCK_SIZE);
              const block = getBlock(index);
              if (!block) break;
              const blockOffset = (position + written) - (index * BLOCK_SIZE);
              const available = Math.min(block.length - blockOffset, count - written);
              if (available <= 0) break;
              result.set(block.subarray(blockOffset, blockOffset + available), written);
              written += available;
            }
            buffer.set(result.subarray(0, written), offset);
            return written;
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
      specialHTMLTargets['#canvas'] = globalThis.pwCanvas;
      log('entry: packages mapped, calling pw153_browser_probe');
      const result = Module._pw153_browser_probe();
      log('return: pw153_browser_probe returned ' + result);
    } catch (error) {
      log('EXCEPTION: ' + (error.stack || error));
    }
  }
};
