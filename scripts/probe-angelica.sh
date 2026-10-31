#!/usr/bin/env bash
# Opt-in local research. Never invoked by the default build or CI.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
REF="references/client-153"
COMMON="$REF/Angelica2/AngelicaCommon"
OUT="artifacts/angelica-probe"
CXX="${CXX:-c++}"
for tool in "$CXX" em++ python3 git; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "Missing tool: $tool. Activate Emscripten before running this local probe." >&2
    exit 1
  fi
done
for file in Source/ARandomGen.cpp Header/ARandomGen.h Header/ABaseDef.h; do
  if ! test -f "$COMMON/$file"; then
    echo "Missing reference file: $COMMON/$file. See docs/POC-ANGELICA.md." >&2
    exit 1
  fi
done
mkdir -p "$OUT"
FLAGS=(-std=c++17 -O2 -Wall -Wextra -Werror -I"$COMMON/Header")
SOURCES=(tests/probes/angelica_random.cpp "$COMMON/Source/ARandomGen.cpp")
"$CXX" "${FLAGS[@]}" "${SOURCES[@]}" -o "$OUT/native-probe"
"$OUT/native-probe" > "$OUT/native.json"
em++ "${FLAGS[@]}" "${SOURCES[@]}" \
  -sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=web \
  -sFILESYSTEM=0 -sEXIT_RUNTIME=1 -sASSERTIONS=1 \
  -o "$OUT/probe.mjs"
cp tests/probes/index.html tests/probes/app.mjs tests/probes/compare.mjs "$OUT/"
python3 - "$CXX" <<'PY'
import hashlib
import json
from pathlib import Path
import subprocess
import sys

root = Path.cwd()
ref = root / 'references/client-153'
out = root / 'artifacts/angelica-probe'
inputs = [root / 'tests/probes/angelica_random.cpp'] + [
    ref / 'Angelica2/AngelicaCommon' / name
    for name in ('Source/ARandomGen.cpp', 'Header/ARandomGen.h', 'Header/ABaseDef.h')
]
def command(*args):
    return subprocess.check_output(args, text=True).strip()

manifest = {
    'reference_commit': command('git', '-C', str(ref), 'rev-parse', 'HEAD'),
    'native_compiler': command(sys.argv[1], '--version').splitlines()[0],
    'emscripten': command('em++', '--version').splitlines()[0],
    'sha256': {str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest()
               for path in inputs + [out / 'native.json', out / 'probe.wasm']},
    'wasm_bytes': (out / 'probe.wasm').stat().st_size,
    'scope': 'ARandomGen only; no Angelica renderer, game assets, networking or gameplay',
}
(out / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
print(json.dumps({'native': 'PASS', 'wasm_build': 'PASS', 'wasm_bytes': manifest['wasm_bytes']}))
PY
echo "Browser parity still needs to run:"
echo "python3 -m http.server 8481 --bind 127.0.0.1 --directory $OUT"
echo "Open http://127.0.0.1:8481/ (local research artifacts; do not publish)."
