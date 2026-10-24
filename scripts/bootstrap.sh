#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
echo "== PW Web bootstrap =="
echo "Root: $ROOT"
if ! command -v git >/dev/null 2>&1; then echo "ERROR: install Git"; exit 1; fi
if ! command -v python3 >/dev/null 2>&1; then echo "ERROR: install Python 3"; exit 1; fi
git --version
python3 --version
if command -v c++ >/dev/null 2>&1; then c++ --version | head -n 1; else echo "NOTICE: no native C++ compiler (native smoke will be skipped)"; fi
if command -v emcc >/dev/null 2>&1; then echo "Emscripten: $(emcc --version | head -n 1)"; else echo "NOTICE: emcc not installed; WASM build is optional"; fi
echo "References (optional; not checked out by bootstrap):"
git submodule status || true
echo "Running verification..."
bash "$ROOT/scripts/verify.sh"
echo "Ready. Start with: ./scripts/dev.sh"
