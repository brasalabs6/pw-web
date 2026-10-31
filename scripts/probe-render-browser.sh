#!/usr/bin/env bash
# Opt-in WebGL2 R0 proof; no asset, reference, or original Angelica build required.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
: "${CHROME_BIN:?Set CHROME_BIN to an installed Chrome/Chromium binary}"
: "${PW_BROWSER_MODULE_DIR:?Set PW_BROWSER_MODULE_DIR to an existing playwright-core installation parent}"
for tool in node python3 curl; do
  command -v "$tool" >/dev/null 2>&1 || { echo "Missing tool: $tool" >&2; exit 2; }
done
[ -x "$CHROME_BIN" ] || { echo "CHROME_BIN is not executable" >&2; exit 2; }
[ -f "$PW_BROWSER_MODULE_DIR/node_modules/playwright-core/package.json" ] ||
  { echo "Missing playwright-core under PW_BROWSER_MODULE_DIR/node_modules/" >&2; exit 2; }
OUT="$ROOT/artifacts/render-probe"
PORT="${PW_RENDER_PORT:-8482}"
for item in index.html render-probe.mjs render-probe.wasm; do
  [ -s "$OUT/$item" ] ||
    { echo "Missing $item: first run ./scripts/build-render-probe.sh" >&2; exit 2; }
done
# Check TCP bind, not just an HTTP response: never take over an existing service.
python3 - <<'PY'
import os
import socket
port = int(os.environ.get("PW_RENDER_PORT", "8482"))
with socket.socket() as sock:
    try:
        sock.bind(("127.0.0.1", port))
    except OSError as exc:
        raise SystemExit(f"Port {port} in use: {exc}")
PY
python3 -m http.server "$PORT" --bind 127.0.0.1 --directory "$OUT" \
  > "$OUT/browser-http.log" 2>&1 &
SERVER_PID=$!
trap 'kill "$SERVER_PID" 2>/dev/null || true; wait "$SERVER_PID" 2>/dev/null || true' EXIT
READY=0
for i in $(seq 1 50); do
  kill -0 "$SERVER_PID" 2>/dev/null ||
    { echo "R0 local HTTP server exited" >&2; exit 1; }
  if curl --max-time 1 -fsS "http://127.0.0.1:$PORT/" >/dev/null 2>&1; then
    READY=1; break
  fi
  sleep 0.2
done
[ "$READY" -eq 1 ] || { echo "R0 local host failed to start" >&2; exit 1; }
export PW_RENDER_PROBE_DIR="$OUT"
node tests/probes/render-browser-proof.cjs
echo "Manifest and screenshots: $OUT/browser-proof.json (local only)"
