#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
CXX="${CXX:-c++}"
for tool in "$CXX" em++ node; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "Missing tool: $tool. Activate Emscripten before testing both targets." >&2
    exit 1
  fi
done
OUT="$(mktemp -d "${TMPDIR:-/tmp}/pw-render-clip.XXXXXXXX")"
trap 'rm -rf "$OUT"' EXIT
FLAGS=(-std=c++17 -O2 -Wall -Wextra -Werror -pedantic -Iengine/render)
SOURCES=(engine/render/clip.cpp tests/render_clip.cpp)
"$CXX" "${FLAGS[@]}" "${SOURCES[@]}" -o "$OUT/native"
"$OUT/native" > "$OUT/native.txt"
em++ "${FLAGS[@]}" "${SOURCES[@]}" -sENVIRONMENT=node -sEXIT_RUNTIME=1 \
  -sASSERTIONS=1 -sFILESYSTEM=0 -o "$OUT/wasm.cjs"
node "$OUT/wasm.cjs" > "$OUT/wasm.txt"
for target in native wasm; do
  if [[ "$(cat "$OUT/$target.txt")" != 'PASS render-clip: 14 cases' ]]; then
    echo "Unexpected fixture result ($target)" >&2
    cat "$OUT/$target.txt" >&2
    exit 1
  fi
  echo "$target: $(cat "$OUT/$target.txt")"
done
cmp "$OUT/native.txt" "$OUT/wasm.txt"
