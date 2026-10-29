#!/usr/bin/env python3
"""Range-capable probe server for the original client entry (m-boot).

Serves the browser entry module (build/client153-entry-browser/), the probe
page/worker, the package manifest, and the client's real packages with HTTP
Range support so the worker's read-only MEMFS streams can seek the multi-GB
.pck/.pkx files on demand without copying them anywhere.

Usage: client153-entry-server.py [port] [--root DIR]
  port    default 8497
  --root  the browser-entry build dir (default build/client153-entry-browser)
"""
import argparse
import json
import os
import re
import socketserver
import stat
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
]

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


class ProbeHandler(SimpleHTTPRequestHandler):
    protocol_version = 'HTTP/1.1'

    def send_bytes(self, data, content_type):
        self.send_response(200)
        self.send_header('Content-Type', content_type)
        self.send_header('Content-Length', str(len(data)))
        self.end_headers()
        self.wfile.write(data)

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
        if route == '/' or route == '/index.html':
            with open(os.path.join(self.root, 'index.html'), 'rb') as handle:
                self.send_bytes(handle.read(), 'text/html; charset=utf-8')
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
            for disk_path, vfs_name in LOOSE_FILES:
                source = os.path.join(ELEMENT_DIR, disk_path)
                if not os.path.isfile(source) or vfs_name in seen_vfs:
                    continue
                seen_vfs.add(vfs_name)
                packages.append({
                    'name': vfs_name,
                    'size': os.path.getsize(source),
                    'url': '/loose/' + disk_path.replace('/', '_'),
                })
            self.send_bytes(
                json.dumps({'packages': packages}).encode(),
                'application/json')
        elif route.startswith('/loose/'):
            # Serve loose files from the element directory.
            disk_name = urllib.parse.unquote(route[len('/loose/'):]).replace('_', '/')
            target = os.path.join(ELEMENT_DIR, disk_name)
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
    args = parser.parse_args()
    ProbeHandler.root = os.path.abspath(args.root)
    for url_name, (disk_path, _vfs) in sorted(PACKAGE_ROUTES.items()):
        source = os.path.join(ELEMENT_DIR, disk_path)
        if not os.path.isfile(source):
            print(f'missing package: {disk_path}')
    server = ThreadingServer(('127.0.0.1', args.port), ProbeHandler)
    print(f'probe server on http://127.0.0.1:{args.port}/ root={ProbeHandler.root}')
    server.serve_forever()


if __name__ == '__main__':
    main()
