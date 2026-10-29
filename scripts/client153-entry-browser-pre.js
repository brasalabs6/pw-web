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
        // The AFPlatform.h separator is '/' on POSIX (the Codex changed it
        // from '\\'), so the client constructs paths with '/'. The MEMFS
        // files must be at forward-slash paths to match. Convert the VFS
        // name's backslashes to forward slashes and create the directory
        // structure in MEMFS.
        const memfsPath = '/' + item.name.replace(/\\/g, '/');
        const dirPath = memfsPath.substring(0, memfsPath.lastIndexOf('/'));
        if (dirPath && dirPath !== '/') {
          try { FS.mkdirTree(dirPath); } catch (e) { /* already exists */ }
        }
        FS.createDataFile(dirPath || '/', memfsPath.substring(memfsPath.lastIndexOf('/') + 1), null, true, false);
        const node = FS.lookupPath(memfsPath).node;
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
      //\tDiagnostic: verify the files are accessible in MEMFS at the
      //\tforward-slash paths.
      try {
        const data = FS.readFile('/help/script.pck');
        log('VFS check: /help/script.pck exists, size=' + data.length);
      } catch (e) { log('VFS check: /help/script.pck FAILS: ' + e.message); }
      try {
        const data = FS.readFile('/configs.pck');
        log('VFS check: /configs.pck exists, size=' + data.length);
      } catch (e) { log('VFS check: /configs.pck FAILS: ' + e.message); }
      //\tFont file check: the CheckImportantFiles fails at index=10 (font\\FZL2JW.ttf).
      try {
        const data = FS.readFile('/font/FZL2JW.ttf');
        log('VFS font check: /font/FZL2JW.ttf exists, size=' + data.length);
      } catch (e) { log('VFS font check: /font/FZL2JW.ttf FAILS: ' + e.message); }
      try {
        const data = FS.readFile('/font/fzl2jw.ttf');
        log('VFS font check: /font/fzl2jw.ttf (lowercase) exists, size=' + data.length);
      } catch (e) { log('VFS font check: /font/fzl2jw.ttf (lowercase) FAILS: ' + e.message); }
      log('entry: packages mapped, calling pw153_browser_probe');
      //\tDiagnostic: test if the canvas's WebGL2 context can produce visible
      //\toutput with a direct JavaScript clear (bypassing the D3D8 adapter).
      const testGl = globalThis.pwCanvas.getContext('webgl2');
      if (testGl) {
        testGl.clearColor(1.0, 0.0, 0.0, 1.0);
        testGl.clear(testGl.COLOR_BUFFER_BIT);
        log('diagnostic: canvas cleared to RED via direct JS WebGL2 call');
      } else {
        log('diagnostic: could not get webgl2 context from canvas');
      }
      const result = Module._pw153_browser_probe();
      log('return: pw153_browser_probe returned ' + result);
      //\tSecond diagnostic: clear to GREEN after the probe returns. If the
      //\tcanvas shows green, the canvas still works and the client's
      //\trendering just doesn't produce visible output.
      const gl2 = globalThis.pwCanvas.getContext('webgl2');
      if (gl2) {
        gl2.clearColor(0.0, 1.0, 0.0, 1.0);
        gl2.clear(gl2.COLOR_BUFFER_BIT);
        log('diagnostic: canvas cleared to GREEN after probe returned');
      }
      // Dump the client's own logs for boot diagnosis.
      try {
        const logDir = FS.readdir('/logs');
        log('logs dir contents: ' + JSON.stringify(logDir));
      } catch (e) { log('no /logs dir: ' + e.message); }
      for (const name of ['AF.log', 'EC.log']) {
        try {
          const text = FS.readFile('/logs/' + name, {encoding: 'utf8'});
          log('=== ' + name + ' ===\n' + text);
        } catch (e) { log('no /logs/' + name + ': ' + e.message); }
      }
    } catch (error) {
      log('EXCEPTION: ' + (error.stack || error));
    }
  }
};
