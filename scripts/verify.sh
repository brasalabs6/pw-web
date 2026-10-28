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
else
  echo "SKIP: native compiler unavailable (CI has C++)"
fi
echo "== server153 CreateRole wire conformance (PWBR v145 client) =="
set +e
scripts/test-server153-wire.sh
wire_rc=$?
set -e
if [ "$wire_rc" -eq 2 ]; then
  echo "SKIP: 32-bit toolchain unavailable (server153 CI image has gcc-multilib)"
elif [ "$wire_rc" -ne 0 ]; then
  echo "wire conformance test FAILED (rc=$wire_rc)" >&2
  exit 1
fi
echo "All available checks passed."
