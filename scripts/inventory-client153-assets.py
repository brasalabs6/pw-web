#!/usr/bin/env python3
"""Inventory authorized local PCK/PKX inputs without extracting or changing them."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools/assets'))
from pck153 import Package


def sha256(path):
    digest = hashlib.sha256()
    with path.open('rb') as source:
        for block in iter(lambda: source.read(4 * 1024 * 1024), b''):
            digest.update(block)
    return digest.hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--client-root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    source_root = args.client_root.resolve()
    output = args.output.resolve()
    if output == source_root or source_root in output.parents:
        parser.error('inventory output must be outside original client')
    records = []
    for source in sorted((source_root / 'element').glob('*.pck')):
        with Package(source) as archive:
            files = [source]
            if source.with_suffix('.pkx').exists():
                files.append(source.with_suffix('.pkx'))
            record = {'package': source.name, 'version': archive.version, 'entryCount': len(archive.entries),
                      'extensions': dict(sorted(Counter(Path(e.path).suffix.lower() for e in archive.entries).items())),
                      'files': [{'name': p.name, 'bytes': p.stat().st_size, 'sha256': sha256(p)} for p in files]}
            records.append(record)
            print(f'{source.name}: {record["entryCount"]} entries, hashes verified')
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps({'schema': 1, 'sourceClient': '1.5.3-v145-pwbr', 'packages': records}, ensure_ascii=False, indent=2) + '\n')


if __name__ == '__main__':
    main()
