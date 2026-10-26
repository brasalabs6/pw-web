import importlib.util
import pathlib
import struct
import sys
import tempfile
import unittest
import zlib

MODULE = pathlib.Path(__file__).resolve().parents[2] / 'tools/assets/pck153.py'
spec = importlib.util.spec_from_file_location('pck153', MODULE)
pck = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = pck
spec.loader.exec_module(pck)


def package(name=b'models/test.mox', data=b'MOXB' * 20):
    payload = zlib.compress(data)
    raw = struct.pack('<260sIIII', name, 12, len(data), len(payload), 0)
    entry = zlib.compress(raw)
    index = struct.pack('<II', len(entry) ^ 0xA8937462, len(entry) ^ 0xA8937462 ^ 0x59374231) + entry
    offset = 12 + len(payload)
    header = struct.pack('<IIII252sI', 0xFDFDFEEE, 0x20002, offset ^ 0xA8937462, 0,
                         b'Angelica File Package', 0xF00DBEEF)
    size = 12 + len(payload) + len(index) + len(header) + 8
    return struct.pack('<III', 0x4DCA23EF, size, 0x56A089B7) + payload + index + header + struct.pack('<II', 1, 0x20002)


class PackageTests(unittest.TestCase):
    def test_read_original_layout_and_payload(self):
        with tempfile.TemporaryDirectory() as root:
            path = pathlib.Path(root) / 'models.pck'
            path.write_bytes(package())
            with pck.Package(path) as archive:
                self.assertEqual(archive.version, 0x20002)
                self.assertEqual(archive.entries[0].path, 'models/test.mox')
                self.assertEqual(archive.read(archive.entries[0]), b'MOXB' * 20)

    def test_reject_truncated_header_and_invalid_entry_check(self):
        with tempfile.TemporaryDirectory() as root:
            path = pathlib.Path(root) / 'models.pck'
            for data in [package()[:-1], b'bad']:
                path.write_bytes(data)
                with self.assertRaises(pck.FormatError):
                    pck.Package(path)
            data = bytearray(package())
            payload_size = len(zlib.compress(b'MOXB' * 20))
            data[12 + payload_size + 4] ^= 1
            path.write_bytes(data)
            with self.assertRaisesRegex(pck.FormatError, 'check'):
                pck.Package(path)

    def test_reject_paths_that_escape_asset_root(self):
        with tempfile.TemporaryDirectory() as root:
            path = pathlib.Path(root) / 'models.pck'
            for name in [b'../outside', b'models/../../outside', b'/absolute', b'C:/absolute']:
                path.write_bytes(package(name))
                with self.assertRaises(pck.FormatError):
                    pck.Package(path)

    def test_split_package_reads_across_real_boundary(self):
        with tempfile.TemporaryDirectory() as root:
            path = pathlib.Path(root) / 'models.pck'
            with path.open('wb') as f:
                f.truncate(pck.SPLIT_BYTES)
                f.seek(pck.SPLIT_BYTES - 2)
                f.write(b'ab')
            path.with_suffix('.pkx').write_bytes(b'cd')
            with pck.LogicalFile(path) as stream:
                self.assertEqual(stream.read_at(pck.SPLIT_BYTES - 2, 4), b'abcd')
                with self.assertRaises(pck.FormatError):
                    stream.read_at(pck.SPLIT_BYTES, 3)


if __name__ == '__main__':
    unittest.main()
