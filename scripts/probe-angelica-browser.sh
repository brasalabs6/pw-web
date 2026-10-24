#!/usr/bin/env bash
# Local opt-in follow-up to scripts/probe-angelica.sh. No copyright-bearing upload.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
: "${CHROME_BIN:?Point CHROME_BIN at a trusted Chrome for Testing binary}"
: "${PW_BROWSER_MODULE_DIR:?Point PW_BROWSER_MODULE_DIR at a local playwright-core install}"
OUT="$ROOT/artifacts/angelica-probe"
for item in index.html app.mjs probe.mjs probe.wasm native.json manifest.json; do
  if [ ! -s "$OUT/$item" ]; then
    echo "Missing $item; first run ./scripts/probe-angelica.sh with authorized reference" >&2
    exit 2
  fi
done
PORT=8481
if curl --max-time 1 -fsS "http://127.0.0.1:$PORT/" >/dev/null 2>&1; then
  echo "Port $PORT is occupied; stop that service before running the local probe" >&2
  exit 2
fi
python3 -m http.server "$PORT" --bind 127.0.0.1 --directory "$OUT" > "$OUT/browser-http.log" 2>&1 &
SERVER_PID=$!
trap 'kill "$SERVER_PID" 2>/dev/null || true' EXIT
READY=0
for i in $(seq 1 50); do
  if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    echo "Local POC HTTP process exited" >&2; exit 1
  fi
  if curl -fsS "http://127.0.0.1:$PORT/" > /dev/null 2>&1; then READY=1; break; fi
  sleep 0.2
done
[ "$READY" -eq 1 ] || { echo "Local POC host failed" >&2; exit 1; }
export PW_ANGELICA_PROBE_DIR="$OUT"
node tests/probes/browser-proof.cjs
echo "Results kept locally at $OUT/browser-proof.json; do not upload without rights review."
