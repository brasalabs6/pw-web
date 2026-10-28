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


def parse_undefined_symbols(stderr: str) -> dict[str, set[str]]:
    seen = {}
    for line in stderr.splitlines():
        if 'undefined symbol: ' not in line:
            continue
        head, _, tail = line.partition('undefined symbol: ')
        symbol = tail.partition(', referenced by ')[0].strip()
        obj = head.split('error: ')[-1].rstrip(': ').strip()
        seen.setdefault(symbol, set()).add(pathlib.Path(obj).name if obj else '?')
    return seen


def format_link_errors(stderr: str) -> str:
    return '\n'.join(line for line in stderr.splitlines()
                     if 'error:' in line and 'undefined symbol:' not in line)


def retention_flags(entry_symbol: str | None) -> list[str]:
    return [f'-Wl,--export={entry_symbol}'] if entry_symbol else ['-Wl,--no-gc-sections']


def node_probe_flags(pre_js: str | None) -> list[str]:
    return ['--pre-js', str(ROOT / 'scripts/client153-readonly-node.js'),
            '--pre-js', pre_js, '-sEXPORTED_FUNCTIONS=_malloc',
            '-sALLOW_MEMORY_GROWTH=1'] if pre_js else []


def browser_probe_flags(pre_js: str | None) -> list[str]:
    # The browser entry probe: the pre-js maps the client packages as
    # Range-request-backed read-only MEMFS streams inside a worker and
    # drives the original wWinMain through pw153_browser_probe. Unlike the
    # Node probe it must not pull the Node-only readonly mapper.
    return ['--pre-js', pre_js, '-sEXPORTED_FUNCTIONS=_malloc,_pw153_browser_probe',
            '-sALLOW_MEMORY_GROWTH=1', '-sMAX_WEBGL_VERSION=2'] if pre_js else []


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
    parser.add_argument('--game-objects', default=str(
        ROOT / 'build/client153-wasm/src/client153/CMakeFiles/'
        'client153_game_original.dir'))
    parser.add_argument('--output', default=str(
        ROOT / 'build/client153-full-link/client153-full-probe.js'))
    parser.add_argument('--error-limit', type=int, default=60)
    parser.add_argument('--entry-symbol', help='retain this mangled symbol and its reachable sections')
    parser.add_argument('--node-probe-pre-js', help='Node-only entry probe script')
    parser.add_argument('--browser-probe-pre-js', help='browser entry probe pre-js (worker context)')
    parser.add_argument('--extra-objects', help='comma-separated extra object files to link')
    parser.add_argument('--list-only', action='store_true')
    args = parser.parse_args()

    objects = sorted(pathlib.Path(args.object_dir).rglob('*.o'))
    engine_dir = pathlib.Path(args.engine_objects)
    if engine_dir.exists():
        objects += sorted(engine_dir.rglob('*.o'))
    gfx_dir = pathlib.Path(args.gfx_objects)
    if gfx_dir.exists():
        objects += sorted(gfx_dir.rglob('*.o'))
    game_dir = pathlib.Path(args.game_objects)
    if game_dir.exists():
        objects += sorted(game_dir.rglob('*.o'))
    if args.extra_objects:
        objects += [pathlib.Path(p) for p in args.extra_objects.split(',')]
    libraries = []
    for name in ('skeleton', 'animation', 'file', 'math', 'common',
                 'platform', 'formats', 'webgl_caps', 'skinning',
                 'speedtree_stub', 'middleware_stub', 'lua', 'glu_tess'):
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
        *node_probe_flags(args.node_probe_pre_js),
        *browser_probe_flags(args.browser_probe_pre_js),
        *retention_flags(args.entry_symbol), f'-Wl,--error-limit={args.error_limit}',
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
    seen = parse_undefined_symbols(result.stderr)
    print(f'distinct undefined symbols: {len(seen)}')
    for symbol in sorted(seen):
        refs = ', '.join(sorted(seen[symbol])[:4])
        print(f'  {symbol}  <-  {refs}')
    other_errors = format_link_errors(result.stderr)
    if other_errors:
        print('other linker errors:')
        print(other_errors)
    return 1


if __name__ == '__main__':
    sys.exit(main())
