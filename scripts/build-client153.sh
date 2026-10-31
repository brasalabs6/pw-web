#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
: "${PW_CLIENT153_ROOT:?Set PW_CLIENT153_ROOT to the authorized client app directory}"
for tool in emcmake cmake ninja python3; do
  command -v "$tool" >/dev/null || { echo "Missing tool: $tool" >&2; exit 1; }
done
emcmake cmake -S . -B build/client153-wasm -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build/client153-wasm
python3 scripts/prepare-client153-scene.py --client-root "$PW_CLIENT153_ROOT" --output build/client153-wasm/browser
printf '%s\n' 'Original asset scene built: build/client153-wasm/browser (gameplay remains incomplete).'
