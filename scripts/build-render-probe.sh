#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
if ! command -v em++ >/dev/null 2>&1; then
  echo 'Missing tool: em++. Activate Emscripten before building the render probe.' >&2
  exit 1
fi
OUT=artifacts/render-probe
mkdir -p "$OUT"
em++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic -Iengine/render \
  engine/render/clip.cpp engine/platform/web/render_probe.cpp --no-entry \
  -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 \
  -sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=web -sFILESYSTEM=0 -sASSERTIONS=1 \
  -sEXPORTED_FUNCTIONS='["_probe_init","_probe_draw","_probe_clear","_probe_pixel","_probe_dispose"]' \
  -o "$OUT/render-probe.mjs"
cp tests/probes/render.html "$OUT/index.html"
echo 'Build complete. Pixel/lifecycle tests still require a browser:'
echo "python3 -m http.server 8482 --bind 127.0.0.1 --directory $OUT"
echo 'Open http://127.0.0.1:8482/ (negative control: /?negative=1).'
