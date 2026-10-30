"""The gshop2.data format the shipped parser must read (B06/PW23 audit
item 4): timestamp, item count, plain GSHOP_ITEM records (1436 bytes,
pack 1, NO per-item retcode — measured on the authorized v145 file,
where item 1's local_id=2 lands at offset 8+1436 and the 8 main-type
names plus main type 5's single sub consume the file to the exact last
byte), then 8 main types (64 wide-char name, sub count, sub names).

Runs against the authorized asset when it is present (a local
authorized file, never committed); CI skips it."""
import os
import struct
import unittest
from pathlib import Path

ASSET = Path(os.environ.get(
    'PW_CLIENT153_GSHOP2',
    '/home/nonexphere/workspace/pw/clients/client-1.5.3-v145-pwbr/'
    'app/element/data/gshop2.data'))
ITEM_SIZE = 1436  # 12 + 128 icon + 8 + 144 (buy 9 fields x 4) + 1024 + 64 + 16 + 32 + 8


def walk(data):
    """Parse with the shipped model; returns (info, ok)."""
    off = 0
    timestamp, = struct.unpack_from('<I', data, off); off += 4
    count, = struct.unpack_from('<i', data, off); off += 4
    items = []
    for _ in range(count):
        if off + ITEM_SIZE > len(data):
            return None, False
        item = data[off:off + ITEM_SIZE]; off += ITEM_SIZE
        local_id, main_type, sub_type = struct.unpack_from('<iii', item, 0)
        icon = item[12:140].split(b'\0')[0]
        items.append((local_id, main_type, sub_type, icon))
    main_types = []
    for _ in range(8):
        if off + 132 > len(data):
            return None, False
        name = data[off:off + 128]; off += 128
        num_sub, = struct.unpack_from('<i', data, off); off += 4
        if num_sub < 0 or num_sub > 64:
            return None, False
        subs = []
        for _ in range(num_sub):
            if off + 128 > len(data):
                return None, False
            subs.append(data[off:off + 128]); off += 128
        main_types.append((name.decode('utf-16-le', 'replace').split('\x00')[0], subs))
    return {'timestamp': timestamp, 'items': items, 'main_types': main_types,
            'end': off, 'size': len(data)}, True


@unittest.skipUnless(ASSET.is_file(), f'authorized gshop2.data not present ({ASSET})')
class ShippedFormat(unittest.TestCase):
    def test_model_consumes_the_authorized_file_exactly(self):
        data = ASSET.read_bytes()
        self.assertEqual(len(data), 4064)
        info, ok = walk(data)
        self.assertTrue(ok)
        self.assertEqual(info['end'], info['size'], 'the model must consume the file exactly')
        self.assertEqual(info['timestamp'], 0x575122b6)  # 2016-06-03 exporter
        # The two point-shop items (pinned from the authorized file).
        self.assertEqual([(i[0], i[1], i[2]) for i in info['items']],
                         [(1, 5, 0), (2, 5, 0)])
        self.assertTrue(info['items'][0][3].startswith(b'Surfaces\\'))
        self.assertTrue(info['items'][1][3].startswith(b'Surfaces\\'))
        # The PWBR shop categories (pinned).
        names = [name for name, _ in info['main_types']]
        self.assertEqual([n.strip() for n in names],
                         ['Especial', 'Roupas', 'Fogos', 'Voo', 'Montaria',
                          'Pontos', 'Aux.', 'ETC'])
        # Main type 5 (Pontos) carries the single sub.
        subs = {i: s for i, (_, s) in enumerate(info['main_types'])}
        self.assertEqual(len(subs[5]), 1)
        self.assertEqual(subs[5][0].decode('utf-16-le', 'replace').split('\x00')[0].strip(),
                         'Pontos')
        for i, (name, sub_list) in enumerate(info['main_types']):
            if i != 5:
                self.assertEqual(len(sub_list), 0, name)

    def test_truncated_file_is_rejected(self):
        data = ASSET.read_bytes()[:-100]
        info, ok = walk(data)
        self.assertFalse(ok, 'a truncated file must not parse')


if __name__ == '__main__':
    unittest.main()
