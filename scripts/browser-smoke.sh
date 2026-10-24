#!/usr/bin/env bash
# Run deterministic Chrome WebGL canvas smoke. Requires external CHROME_BIN + Playwright-core.
set -euo pipefail
: "${CHROME_BIN:?CHROME_BIN is required}"
: "${PW_BROWSER_MODULE_DIR:?PW_BROWSER_MODULE_DIR is required}"
PORT="${PW_WEB_PORT:-8480}"
if ! [[ "$PORT" =~ ^[0-9]+$ ]] || (( PORT < 1024 || PORT > 65535 )); then echo "Invalid browser port: $PORT" >&2; exit 2; fi
export PW_BROWSER_URL="http://127.0.0.1:$PORT/"
OUT="${PW_BROWSER_ARTIFACTS:-artifacts/browser}"
mkdir -p "$OUT"
python3 -m http.server "$PORT" --bind 127.0.0.1 --directory web > "$OUT/http.log" 2>&1 &
SERVER_PID=$!
trap 'kill "$SERVER_PID" 2>/dev/null || true' EXIT
for i in $(seq 1 30); do
  if curl -fsS $PW_BROWSER_URL > /dev/null; then break; fi
  sleep 0.2
done
if ! kill -0 "$SERVER_PID" 2>/dev/null; then echo "Browser server exited unexpectedly" >&2; exit 1; fi
node tests/browser_smoke.cjs
