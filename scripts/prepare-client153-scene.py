#!/usr/bin/env python3
"""Stage an original MOX and its sibling DDS resources, never the whole PCK."""
import argparse
import hashlib
import json
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools/assets'))
from pck153 import Package

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--client-root', type=Path, required=True)
parser.add_argument('--model', default='models/home/物品/柜立柜/柜立柜.mox')
parser.add_argument('--output', type=Path, default=Path('artifacts/client153-browser'))
args = parser.parse_args()
input_root = args.client_root.resolve()
root = input_root / 'element'
output = args.output.resolve()
if output == input_root or input_root in output.parents:
    parser.error('output must be separate from original client')
output.mkdir(parents=True, exist_ok=True)
with Package(root / 'models.pck') as package:
    matches = [entry for entry in package.entries if entry.path.casefold() == args.model.casefold()]
    if len(matches) != 1:
        parser.error('model must resolve to exactly one original package entry')
    entry = matches[0]
    directory = entry.path.rsplit('/', 1)[0]
    selected = [entry] + [e for e in package.entries if e.path.casefold().startswith((directory + '/textures/').casefold()) and e.path.lower().endswith('.dds')]
    manifest = {'schema': 1, 'sourceClient': '1.5.3-v145-pwbr', 'upstreamCommit': 'e3d6d0075d0a745c9f9859f2cb25b01d71dec045',
                'scope': 'original MOX resource scene; gameplay incomplete', 'textures': {}}
    for item in selected:
        content = package.read(item)
        digest = hashlib.sha256(content).hexdigest()
        filename = digest + Path(item.path).suffix.lower()
        (output / filename).write_bytes(content)
        record = {'path': item.path, 'url': filename, 'bytes': len(content), 'sha256': digest}
        if item is entry:
            manifest['model'] = record
        else:
            key = item.path.rsplit('/', 1)[-1].lower()
            if key in manifest['textures']:
                raise ValueError('ambiguous texture name')
            manifest['textures'][key] = record
    (output / 'scene.json').write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({'model': entry.path, 'objects': len(selected), 'bytes': sum((output / r['url']).stat().st_size for r in [manifest['model'], *manifest['textures'].values()])}, ensure_ascii=False))
