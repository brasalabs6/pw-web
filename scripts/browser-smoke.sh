#!/usr/bin/env bash
# Run deterministic Chrome WebGL canvas smoke. Requires external CHROME_BIN + Playwright-core.
set -euo pipefail
: "${CHROME_BIN:?CHROME_BIN is required}"
: "${PW_BROWSER_MODULE_DIR:?PW_BROWSER_MODULE_DIR is required}"
OUT="${PW_BROWSER_ARTIFACTS:-artifacts/browser}"
mkdir -p "$OUT"
python3 -m http.server 8080 --bind 127.0.0.1 --directory web > "$OUT/http.log" 2>&1 &
SERVER_PID=$!
trap 'kill "$SERVER_PID" 2>/dev/null || true' EXIT
for i in $(seq 1 30); do
  if curl -fsS http://127.0.0.1:8080/ > /dev/null; then break; fi
  sleep 0.2
done
node tests/browser_smoke.cjs
