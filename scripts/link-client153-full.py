#!/usr/bin/env python3
"""Link every object the broad probe compiled into one WASM module.

The m-link diagnostic: takes all real game/AUI/engine objects from the
probe's object directory (scripts/probe-client153-project.py --object-dir)
plus the port static libraries and links them with Emscripten. The
unresolved-symbol list is the m-link worklist (the TUs that do not
compile yet, and the port seams still missing implementations).

Usage: link-client153-full.py [--object-dir DIR] [--wasm-build DIR]
                               [--output FILE] [--error-limit N] [--list-only]
"""
import argparse
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument('--emxx', default=str(
        pathlib.Path.home() /
        '.local/share/pw-toolchains/emsdk/upstream/emscripten/em++'))
    parser.add_argument('--object-dir', default=str(
        ROOT / 'build/client153-original-objects-clean'))
    parser.add_argument('--wasm-build', default=str(
        ROOT / 'build/client153-wasm/src/client153'))
    parser.add_argument('--engine-objects', default=str(
        ROOT / 'build/client153-wasm/src/client153/CMakeFiles/'
        'client153_engine_original.dir'))
    parser.add_argument('--gfx-objects', default=str(
        ROOT / 'build/client153-wasm/src/client153/CMakeFiles/'
        'client153_gfx_original.dir'))
    parser.add_argument('--output', default=str(
        ROOT / 'build/client153-full-link/client153-full-probe.js'))
    parser.add_argument('--error-limit', type=int, default=60)
    parser.add_argument('--list-only', action='store_true')
    args = parser.parse_args()

    objects = sorted(pathlib.Path(args.object_dir).rglob('*.o'))
    engine_dir = pathlib.Path(args.engine_objects)
    if engine_dir.exists():
        objects += sorted(engine_dir.rglob('*.o'))
    gfx_dir = pathlib.Path(args.gfx_objects)
    if gfx_dir.exists():
        objects += sorted(gfx_dir.rglob('*.o'))
    libraries = []
    for name in ('skeleton', 'animation', 'file', 'math', 'common',
                 'platform', 'formats', 'webgl_caps', 'skinning',
                 'speedtree_stub'):
        archive = pathlib.Path(args.wasm_build) / f'libclient153_{name}.a'
        if archive.exists():
            libraries.append(str(archive))
    print(f'objects: {len(objects)}  libraries: {len(libraries)}')
    if args.list_only:
        for obj in objects:
            print(' ', obj.relative_to(ROOT))
        return 0

    output = pathlib.Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    command = [
        args.emxx, '--no-entry', '-fexceptions', '-sUSE_ZLIB=1',
        '-Wl,--no-gc-sections', f'-Wl,--error-limit={args.error_limit}',
        *objects, *libraries, '-o', str(output),
    ]
    print('linking...')
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode == 0:
        print(f'LINK OK: {output}')
        return 0
    undefined = []
    for line in result.stderr.splitlines():
        if 'undefined symbol' in line:
            undefined.append(line.strip())
    print(f'link failed: {len(undefined)} undefined-symbol reports')
    seen = {}
    for line in undefined:
        # wasm-ld: error: <object>: undefined symbol: <name>, referenced by <ref>
        head, _, tail = line.partition('undefined symbol: ')
        symbol = tail.split(',')[0].strip()
        obj = head.split('error: ')[-1].split(': undefined')[0].strip()
        obj = pathlib.Path(obj).name if obj else '?'
        ref = tail.split('referenced by ')[-1].strip() if 'referenced by ' in tail else ''
        seen.setdefault(symbol, set()).add(obj)
    print(f'distinct undefined symbols: {len(seen)}')
    for symbol in sorted(seen):
        refs = ', '.join(sorted(seen[symbol])[:4])
        print(f'  {symbol}  <-  {refs}')
    return 1


if __name__ == '__main__':
    sys.exit(main())
