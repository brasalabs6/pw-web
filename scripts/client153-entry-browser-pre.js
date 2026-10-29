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
  onRuntimeInitialized: async function() {
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
      // Loose (non-package) files must be provisioned eagerly with their
      // real bytes: the sync-XHR text path cannot transport binary files
      // that start with a UTF-16 BOM (ff fe) — the Encoding Standard's BOM
      // sniffing decodes them as UTF-16LE regardless of the declared
      // charset, consuming the BOM and collapsing the 2-byte units (this
      // corrupted world_targets.txt and serverlist.txt). fetch() with
      // arrayBuffer has no text decoding, so it preserves every byte.
      // The multi-GB .pck/.pkx packages keep the lazy Range-request
      // streams: their binary headers never trigger BOM sniffing.
      const looseFiles = [];
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
        if (!/\.(pck|pkx)$/i.test(item.name)) { looseFiles.push({ item, memfsPath }); continue; }
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
      // Provision the loose files with real bytes via async fetch (no text
      // decoding, byte-exact). This must complete before the probe runs
      // because the client reads these files synchronously during boot.
      await Promise.all(looseFiles.map(async ({ item, memfsPath }) => {
        try {
          const response = await fetch(item.url);
          if (!response.ok) throw Error('HTTP ' + response.status + ' ' + item.url);
          const bytes = new Uint8Array(await response.arrayBuffer());
          if (bytes.length !== item.size) log('loose size mismatch ' + item.name + ': got ' + bytes.length + ' expected ' + item.size);
          FS.writeFile(memfsPath, bytes);
        } catch (e) {
          log('loose fetch failed ' + item.name + ': ' + e.message);
        }
      }));
      // The client opens userdata\SystemSettings.ini, which AFile resolves
      // to /userdata/SystemSettings.ini in MEMFS (forward slashes). The ini
      // must exist with FullScreen=0: the browser device only supports
      // windowed mode, and the client's default is fullscreen.
      try { FS.mkdir('/userdata'); } catch(e) { /* already exists */ }
      FS.writeFile('/userdata/SystemSettings.ini', '[Video]\nFullScreen=0\n');
      try { FS.writeFile('/client_id.cfg', '[Default]\nclient_id=1\n'); } catch (e) { log('client_id.cfg write failed: ' + e.message); }
      // The ALog system creates a "Logs" directory for the log files. The
      // MEMFS needs this directory to exist before the client boots.
      try { FS.mkdir('/Logs'); } catch(e) { /* already exists */ }
      try { FS.mkdir('/logs'); } catch(e) { /* already exists */ }
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
      const result = Module._pw153_browser_probe();
      log('return: pw153_browser_probe returned ' + result);
      // Dump the client's own logs for boot diagnosis. ALog uses the "Logs"
      // directory (capital L, case-sensitive MEMFS) and names A3D.log (engine
      // errlog), AM.log (media), EC.log (element client), AF.log (file system).
      for (const dir of ['/Logs', '/logs']) {
        try {
          const logDir = FS.readdir(dir);
          log(dir + ' dir contents: ' + JSON.stringify(logDir));
        } catch (e) { log('no ' + dir + ' dir: ' + e.message); }
        for (const name of ['A3D.log', 'AM.log', 'EC.log', 'AF.log']) {
          try {
            let text = FS.readFile(dir + '/' + name, {encoding: 'utf8'});
            const lines = text.split('\n');
            if (lines.length > 300) {
              text = '... (' + (lines.length - 300) + ' earlier lines)\n' + lines.slice(-300).join('\n');
            }
            log('=== ' + dir + '/' + name + ' ===\n' + text);
          } catch (e) { log('no ' + dir + '/' + name + ': ' + e.message); }
        }
      }
    } catch (error) {
      log('EXCEPTION: ' + (error.stack || error));
    }
  }
};
