#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
if ! command -v emcc >/dev/null 2>&1; then
  echo "Emscripten 'emcc' not found. Install/activate emsdk: https://emscripten.org/docs/getting_started/downloads.html" >&2
  exit 1
fi
mkdir -p web/wasm
emcc engine/core/pw_core.cpp \
  -std=c++17 -O2 -Wall -Wextra -Werror \
  --no-entry \
  -sMODULARIZE=1 \
  -sEXPORT_ES6=1 \
  -sENVIRONMENT=web \
  -sFILESYSTEM=0 \
  -sALLOW_MEMORY_GROWTH=1 \
  "-sEXPORTED_FUNCTIONS=['_pw_core_add','_pw_core_version','_pw_core_read_u32le']" \
  -o web/wasm/pw_core.mjs
echo "Built web/wasm/pw_core.mjs and web/wasm/pw_core.wasm"
echo "Serve with ./scripts/dev.sh and check the WASM diagnostic."
