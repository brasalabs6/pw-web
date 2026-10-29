import importlib.util
import json
import pathlib
import tempfile
import threading
import unittest
import urllib.request
import urllib.error

ROOT = pathlib.Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('entry_server', ROOT / 'scripts/client153-entry-server.py')
server_module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(server_module)

class LooseFontsTest(unittest.TestCase):
    def test_manifest_and_range_preserve_client_font_names(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            (root / 'font').mkdir()
            (root / 'font/fzl2jw.ttf').write_bytes(b'font-data')
            original = server_module.ELEMENT_DIR
            server_module.ELEMENT_DIR = tmp
            server = server_module.ThreadingServer(('127.0.0.1', 0), server_module.ProbeHandler)
            thread = threading.Thread(target=server.serve_forever, daemon=True)
            thread.start()
            try:
                base = f'http://127.0.0.1:{server.server_address[1]}'
                manifest = json.load(urllib.request.urlopen(base + '/manifest'))
                item = next(x for x in manifest['packages'] if x['name'] == 'font\\FZL2JW.ttf')
                req = urllib.request.Request(base + item['url'], headers={'Range': 'bytes=0-3'})
                with urllib.request.urlopen(req) as response:
                    self.assertEqual(response.status, 206)
                    self.assertEqual(response.read(), b'font')
                (root / 'secret.txt').write_bytes(b'private')
                with self.assertRaises(urllib.error.HTTPError) as rejected:
                    urllib.request.urlopen(base + '/loose/secret.txt')
                self.assertEqual(rejected.exception.code, 404)
                rejected.exception.close()
            finally:
                server.shutdown(); server.server_close(); thread.join()
                server_module.ELEMENT_DIR = original

if __name__ == '__main__':
    unittest.main()
