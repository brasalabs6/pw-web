#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
echo "== shell syntax =="
for f in scripts/*.sh; do bash -n "$f"; done
echo "== standard-library Python tests =="
python3 -m unittest discover -s tests -p 'test_*.py' -v
echo "== portable C++ core =="
if command -v c++ >/dev/null 2>&1; then
  BIN="$(mktemp "${TMPDIR:-/tmp}/pw-web-smoke.XXXXXXXX")"
  trap 'rm -f "$BIN"' EXIT
  c++ -std=c++17 -Wall -Wextra -Werror -pedantic -Iengine/core \
    engine/core/pw_core.cpp tests/core_smoke.cpp -o "$BIN"
  "$BIN"
  echo "== portable renderer contract =="
  c++ -std=c++17 -Wall -Wextra -Werror -pedantic -Iengine/render \
    engine/render/clip.cpp tests/render_clip.cpp -o "$BIN"
  "$BIN"
else
  echo "SKIP: native compiler unavailable (CI has C++)"
fi
echo "All available checks passed."
