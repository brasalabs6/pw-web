#!/usr/bin/env python3
"""Range-capable probe server for the original client entry (m-boot).

Serves the browser entry module (build/client153-entry-browser/), the probe
page/worker, the package manifest, and the client's real packages with HTTP
Range support so the worker's read-only MEMFS streams can seek the multi-GB
.pck/.pkx files on demand without copying them anywhere.

Canonical entry (B01): the matched JS glue + WASM pair is served from the
build's own output directory (--pair-dir, default
build/client153-wasm/src/client153) under the names the link produced, with
Cache-Control: no-store so a hot cache can never pair a new glue with a
stale binary. /build-info reports the pair's SHA-256 hashes and the
worktree's git HEAD; the canonical page (/, /entry) logs them and verifies
the bytes actually served against those hashes. Stale build-dir pages and
glues (entry-r3.js, entry-u04.js, old pair copies) are refused with 410 so
old pages cannot mix releases.

Usage: client153-entry-server.py [port] [--root DIR] [--pair-dir DIR]
  port    default 8497
  --root      the probe/loose-file dir (default build/client153-entry-browser)
  --pair-dir  the build output dir with the matched entry pair
              (default build/client153-wasm/src/client153)
"""
import argparse
import hashlib
import json
import os
import re
import socketserver
import stat
import subprocess
import urllib.parse
from http.server import SimpleHTTPRequestHandler

ELEMENT_DIR = os.path.expanduser(
    '~/workspace/pw/clients/client-1.5.3-v145-pwbr/app/element')

# Loose files the client expects on its VFS root (fonts are NOT inside
# fonts.pck — they are individual .ttf files in element/font/).
LOOSE_FILES = [
    ('font/fzl2jw.ttf', 'font\\FZL2JW.ttf'),
    ('font/fzlbjw.ttf', 'font\\FZLBJW.ttf'),
    ('font/fzxh1jw.ttf', 'font\\FZXH1JW.ttf'),
    ('data/fzxh1jw.ttf', 'data\\fzxh1jw.ttf'),
    ('data/fzxh1jw1.ttf', 'data\\fzxh1jw1.ttf'),
    ('data/verdana.ttf', 'data\\verdana.ttf'),
    ('data/times.ttf', 'data\\times.ttf'),
    ('data/kaiu.ttf', 'data\\kaiu.ttf'),
    ('cursors/normal.ani', 'Cursors\\normal.ani'),
    ('cursors/attack.cur', 'Cursors\\attack.cur'),
    ('cursors/talk.cur', 'Cursors\\talk.cur'),
    ('cursors/pick.cur', 'Cursors\\pick.cur'),
    ('cursors/repair.cur', 'Cursors\\repair.cur'),
    ('cursors/hand.cur', 'Cursors\\hand.cur'),
    ('cursors/flag.cur', 'Cursors\\flag.cur'),
    ('cursors/dig.ani', 'Cursors\\dig.ani'),
    ('cursors/gourd.cur', 'Cursors\\gourd.cur'),
    ('maps/world/world_targets.txt', 'maps\\world\\world_targets.txt'),
    ('../patcher/server/serverlist.txt', 'userdata\\server\\serverlist.txt'),
    ('data/domain.data', 'Data\\domain.data'),
    ('data/domain2.data', 'Data\\domain2.data'),
    ('data/domain2_cross.data', 'Data\\domain2_cross.data'),
    ('data/elements.data', 'Data\\elements.data'),
    ('data/elements.data', 'data\\elements.data'),
    ('data/tasks.data', 'Data\\tasks.data'),
    ('data/tasks.data', 'data\\tasks.data'),
    ('data/dyn_tasks.data', 'Data\\dyn_tasks.data'),
    ('data/aipolicy.data', 'Data\\aipolicy.data'),
    ('data/gshop.data', 'Data\\gshop.data'),
    ('data/gshop1.data', 'Data\\gshop1.data'),
    ('data/gshop2.data', 'Data\\gshop2.data'),
    ('data/task_npc.data', 'Data\\task_npc.data'),
]

def loose_files():
    """Expose the installed login scene alongside the explicitly listed files."""
    files = list(LOOSE_FILES)
    scene_root = os.path.join(ELEMENT_DIR, 'maps', 'login')
    for directory, subdirs, names in os.walk(scene_root, followlinks=False):
        subdirs[:] = sorted(name for name in subdirs
                            if not os.path.islink(os.path.join(directory, name)))
        for name in sorted(names):
            path = os.path.join(directory, name)
            if os.path.islink(path) or not os.path.isfile(path):
                continue
            relative = os.path.relpath(path, scene_root).replace(os.sep, '/')
            vfs_relative = relative
            if relative in ('login.ecwld', 'login.ecbsd', 'login.cld', 'login.trn2'):
                vfs_relative = 'L' + relative[1:]
            elif relative.startswith('login_login.'):
                vfs_relative = 'login_Login.' + relative.split('.', 1)[1]
            files.append(('maps/login/' + relative,
                          'Maps\\Login\\' + vfs_relative.replace('/', '\\')))
    return files

# The packages the original entry expects on its VFS root (the same set the
# Node probe mapped; names must match the client's own lookups — the script
# package uses a backslash in the client's VFS name).
PACKAGES = [
    ('building.pck', 'building.pck'),
    ('configs.pck', 'configs.pck'),
    ('facedata.pck', 'facedata.pck'),
    ('fonts.pck', 'fonts.pck'),
    ('gfx.pck', 'gfx.pck'),
    ('grasses.pck', 'grasses.pck'),
    ('interfaces.pck', 'interfaces.pck'),
    ('litmodels.pck', 'litmodels.pck'),
    ('litmodels.pkx', 'litmodels.pkx'),
    ('loddata.pck', 'loddata.pck'),
    ('models.pck', 'models.pck'),
    ('models.pkx', 'models.pkx'),
    ('sfx.pck', 'sfx.pck'),
    ('shaders.pck', 'shaders.pck'),
    ('surfaces.pck', 'surfaces.pck'),
    ('textures.pck', 'textures.pck'),
    ('trees.pck', 'trees.pck'),
    ('help/script.pck', 'help\\script.pck'),
]
# URL-safe routing name -> (disk path, VFS name)
PACKAGE_ROUTES = {
    'help-script.pck': ('help/script.pck', 'help\\script.pck'),
}
for _disk, _vfs in PACKAGES:
    PACKAGE_ROUTES.setdefault(_vfs, (_disk, _vfs))


# The canonical entry page (B01): loads the matched pair by the names the
# link produced, logs /build-info, and verifies the served bytes against
# the reported hashes so a mixed or cached artifact is visible immediately.
CANONICAL_ENTRY_PAGE = """<!DOCTYPE html>
<html><head><meta charset="utf-8"><title>PW 1.5.3 — canonical entry</title></head>
<body style="margin:0;background:#111">
<canvas id="canvas" width="1024" height="768" style="display:block;margin:0 auto;background:#000"></canvas>
<pre id="status" style="color:#8f8;font:12px monospace;white-space:pre-wrap;margin:8px"></pre>
<script>
window.pwCanvas = document.getElementById('canvas');
window.pwStatus = document.getElementById('status');
(async () => {
  const log = (msg) => { console.log('[pw153]', msg);
    if (window.pwStatus) window.pwStatus.textContent += '\\n' + msg; };
  try {
    const info = await (await fetch('/build-info', {cache: 'no-store'})).json();
    log('build-info: git=' + info.worktree_git_sha +
        ' js=' + info.js_sha256.slice(0, 16) +
        ' wasm=' + info.wasm_sha256.slice(0, 16) +
        ' (js ' + info.js_bytes + ' B, wasm ' + info.wasm_bytes + ' B)');
    const digest = async (url) => {
      const bytes = new Uint8Array(await (await fetch(url, {cache: 'no-store'})).arrayBuffer());
      const hash = await crypto.subtle.digest('SHA-256', bytes);
      return [...new Uint8Array(hash)].map(b => b.toString(16).padStart(2, '0')).join('');
    };
    const jsSha = await digest('/client153_browser_entry.js');
    const wasmSha = await digest('/client153_browser_entry.wasm');
    log('served:   js=' + jsSha.slice(0, 16) + ' wasm=' + wasmSha.slice(0, 16));
    log('pair match: js=' + (jsSha === info.js_sha256) +
        ' wasm=' + (wasmSha === info.wasm_sha256));
  } catch (error) { log('build-info verification failed: ' + error); }
})();
</script>
<script src="/client153_browser_entry.js?v=__JS_SHA8__"></script>
</body></html>
"""


class ProbeHandler(SimpleHTTPRequestHandler):
    protocol_version = 'HTTP/1.1'

    def send_bytes(self, data, content_type):
        self.send_response(200)
        self.send_header('Content-Type', content_type)
        self.send_header('Content-Length', str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    # ---- Canonical entry pair (B01) -------------------------------------
    def pair_files(self):
        """The matched glue/binary pair from the build output dir."""
        return (os.path.join(self.pair_dir, 'client153_browser_entry.js'),
                os.path.join(self.pair_dir, 'client153_browser_entry.wasm'))

    def build_info(self):
        js_path, wasm_path = self.pair_files()
        with open(js_path, 'rb') as handle:
            js_sha = hashlib.sha256(handle.read()).hexdigest()
        with open(wasm_path, 'rb') as handle:
            wasm_bytes = handle.read()
        try:
            git_sha = subprocess.run(
                ['git', '-C', os.path.dirname(os.path.abspath(__file__)) + '/..',
                 'rev-parse', '--short', 'HEAD'],
                capture_output=True, text=True, timeout=5).stdout.strip()
        except (OSError, subprocess.SubprocessError):
            git_sha = 'unknown'
        return {
            'worktree_git_sha': git_sha,
            'js_sha256': js_sha,
            'wasm_sha256': hashlib.sha256(wasm_bytes).hexdigest(),
            'js_bytes': os.path.getsize(js_path),
            'wasm_bytes': os.path.getsize(wasm_path),
        }

    def canonical_page(self):
        info = self.build_info()
        html = CANONICAL_ENTRY_PAGE.replace('__JS_SHA8__', info['js_sha256'][:8])
        self.send_bytes(html.encode(), 'text/html; charset=utf-8')

    def serve_pair_file(self, name):
        path = os.path.join(self.pair_dir, name)
        if not os.path.isfile(path):
            self.send_error(404, 'the entry pair is not built yet')
            return
        self.send_response(200)
        self.send_header('Content-Type',
                         'application/wasm' if name.endswith('.wasm')
                         else 'application/javascript')
        self.send_header('Content-Length', str(os.path.getsize(path)))
        # The pair must never come from a hot cache: a cached binary under
        # a fresh glue is exactly the release-mixing the canonical entry
        # exists to prevent.
        self.send_header('Cache-Control', 'no-store')
        self.end_headers()
        with open(path, 'rb') as handle:
            while True:
                chunk = handle.read(1 << 20)
                if not chunk:
                    break
                self.wfile.write(chunk)

    def serve_file_with_range(self, path):
        # importScripts in a dedicated worker rejects non-JavaScript MIME
        # types, so the build-dir fallback must speak proper MIME.
        extension = os.path.splitext(path)[1].lower()
        content_type = {
            '.js': 'application/javascript',
            '.mjs': 'application/javascript',
            '.wasm': 'application/wasm',
            '.html': 'text/html; charset=utf-8',
            '.json': 'application/json',
        }.get(extension, 'application/octet-stream')
        size = os.path.getsize(path)
        range_header = self.headers.get('Range')
        if not range_header:
            with open(path, 'rb') as handle:
                self.send_response(200)
                self.send_header('Content-Type', content_type)
                self.send_header('Content-Length', str(size))
                self.send_header('Accept-Ranges', 'bytes')
                self.end_headers()
                while True:
                    chunk = handle.read(1 << 20)
                    if not chunk:
                        break
                    self.wfile.write(chunk)
            return
        match = re.match(r'bytes=(\d+)-(\d*)', range_header)
        if not match:
            self.send_error(416)
            return
        start = int(match.group(1))
        end = int(match.group(2)) if match.group(2) else size - 1
        if start >= size or end >= size or start > end:
            self.send_error(416)
            return
        with open(path, 'rb') as handle:
            handle.seek(start)
            data = handle.read(end - start + 1)
        self.send_response(206)
        self.send_header('Content-Type', content_type)
        self.send_header('Content-Range', f'bytes {start}-{end}/{size}')
        self.send_header('Content-Length', str(len(data)))
        self.send_header('Accept-Ranges', 'bytes')
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        parsed = urllib.parse.urlparse(self.path)
        route = parsed.path
        if route == '/' or route == '/entry' or route == '/index.html':
            self.canonical_page()
        elif route == '/build-info':
            self.send_bytes(json.dumps(self.build_info()).encode(),
                            'application/json')
        elif route == '/client153_browser_entry.js':
            self.serve_pair_file('client153_browser_entry.js')
        elif route == '/client153_browser_entry.wasm':
            self.serve_pair_file('client153_browser_entry.wasm')
        elif route == '/worker.js':
            with open(os.path.join(self.root, 'worker.js'), 'rb') as handle:
                self.send_bytes(handle.read(), 'application/javascript')
        elif route == '/manifest':
            packages = []
            seen_vfs = set()
            for url_name, (disk_path, vfs_name) in sorted(PACKAGE_ROUTES.items()):
                if vfs_name in seen_vfs:
                    continue
                source = os.path.join(ELEMENT_DIR, disk_path)
                if not os.path.isfile(source):
                    continue
                seen_vfs.add(vfs_name)
                packages.append({
                    'name': vfs_name,
                    'size': os.path.getsize(source),
                    'url': '/pkg/' + url_name,
                })
            # Loose files (fonts etc.) that live outside any .pck package.
            for disk_path, vfs_name in loose_files():
                source = os.path.join(ELEMENT_DIR, disk_path)
                if not os.path.isfile(source) or vfs_name in seen_vfs:
                    continue
                seen_vfs.add(vfs_name)
                packages.append({
                    'name': vfs_name,
                    'size': os.path.getsize(source),
                    'url': '/loose/' + urllib.parse.quote(disk_path, safe=''),
                })
            self.send_bytes(
                json.dumps({'packages': packages}).encode(),
                'application/json')
        elif route.startswith('/loose/'):
            disk_name = urllib.parse.unquote(route[len('/loose/'):])
            if disk_name not in {disk for disk, _ in loose_files()}:
                self.send_error(404)
                return
            target = os.path.normpath(os.path.join(ELEMENT_DIR, disk_name))
            if not os.path.isfile(target):
                self.send_error(404)
                return
            self.serve_file_with_range(target)
        elif route.startswith('/pkg/'):
            url_name = urllib.parse.unquote(route[len('/pkg/'):])
            if url_name not in PACKAGE_ROUTES:
                self.send_error(404)
                return
            disk_path, _vfs = PACKAGE_ROUTES[url_name]
            self.serve_file_with_range(os.path.join(ELEMENT_DIR, disk_path))
        else:
            # The module files and anything else from the build dir.
            relative = os.path.normpath(route.lstrip('/'))
            if relative.startswith('..'):
                self.send_error(403)
                return
            # B01: stale pages and glues in the probe dir must not be
            # servable — an old glue against the current binary (or the
            # reverse) is the release-mixing this server prevents. The
            # canonical entry is / (or /entry); the pair comes from
            # --pair-dir. Only the worker relay stays reachable.
            extension = os.path.splitext(relative)[1].lower()
            if extension in ('.html', '.js', '.wasm', '.mjs'):
                self.send_error(
                    410, 'stale build-dir page/glue: use the canonical '
                    'entry at / (the matched pair is served from the '
                    'build output dir)')
                return
            target = os.path.join(self.root, relative)
            if not os.path.isfile(target):
                self.send_error(404)
                return
            self.serve_file_with_range(target)


class ThreadingServer(socketserver.ThreadingMixIn,
                       socketserver.TCPServer):
    allow_reuse_address = True
    daemon_threads = True


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('port', nargs='?', type=int, default=8497)
    parser.add_argument('--root', default='build/client153-entry-browser')
    parser.add_argument('--pair-dir', default='build/client153-wasm/src/client153',
                        help='the build output dir with the matched entry pair')
    args = parser.parse_args()
    ProbeHandler.root = os.path.abspath(args.root)
    ProbeHandler.pair_dir = os.path.abspath(args.pair_dir)
    for url_name, (disk_path, _vfs) in sorted(PACKAGE_ROUTES.items()):
        source = os.path.join(ELEMENT_DIR, disk_path)
        if not os.path.isfile(source):
            print(f'missing package: {disk_path}')
    for path in (os.path.join(ProbeHandler.pair_dir, 'client153_browser_entry.js'),
                 os.path.join(ProbeHandler.pair_dir, 'client153_browser_entry.wasm')):
        if not os.path.isfile(path):
            print(f'missing entry pair file: {path}')
    server = ThreadingServer(('127.0.0.1', args.port), ProbeHandler)
    print(f'probe server on http://127.0.0.1:{args.port}/ root={ProbeHandler.root} '
          f'pair={ProbeHandler.pair_dir}')
    server.serve_forever()


if __name__ == '__main__':
    main()
