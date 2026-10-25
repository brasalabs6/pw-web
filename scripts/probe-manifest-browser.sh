#!/usr/bin/env bash
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
PORT="${PW_MANIFEST_PORT:-8486}"
mkdir -p artifacts
STAGE="$(mktemp -d "${TMPDIR:-/tmp}/pw-manifest-browser.XXXXXXXX")"
SERVER_PID=""
cleanup() {
  if [ -n "$SERVER_PID" ]; then
    kill "$SERVER_PID" 2>/dev/null || true
    wait "$SERVER_PID" 2>/dev/null || true
  fi
  rm -f artifacts/manifest-http.log
  rm -rf "$STAGE"
}
trap cleanup EXIT
mkdir -p \
  "$STAGE/tests/contracts" \
  "$STAGE/tests/fixtures/manifests" \
  "$STAGE/tools/contracts" \
  "$STAGE/tools/assets/schemas" \
  "$STAGE/tools/release/schemas"
cp tests/contracts/manifest-browser.html "$STAGE/tests/contracts/"
cp tests/fixtures/manifests/*.json "$STAGE/tests/fixtures/manifests/"
cp tools/contracts/manifest-validator.mjs "$STAGE/tools/contracts/"
cp tools/assets/schemas/content-manifest.schema.json "$STAGE/tools/assets/schemas/"
cp tools/release/schemas/release-manifest.schema.json "$STAGE/tools/release/schemas/"
python3 - <<'PY'
import os
import socket
port = int(os.environ.get("PW_MANIFEST_PORT", "8486"))
with socket.socket() as sock:
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        sock.bind(("127.0.0.1", port))
    except OSError as exc:
        raise SystemExit(f"Port {port} in use: {exc}")
PY
python3 -m http.server "$PORT" --bind 127.0.0.1 --directory "$STAGE" \
  > artifacts/manifest-http.log 2>&1 &
SERVER_PID=$!
READY=0
for _ in $(seq 1 50); do
  kill -0 "$SERVER_PID" 2>/dev/null || { echo "Local manifest server exited" >&2; exit 1; }
  if curl --max-time 1 -fsS "http://127.0.0.1:$PORT/tests/contracts/manifest-browser.html" >/dev/null 2>&1; then
    READY=1
    break
  fi
  sleep 0.2
done
[ "$READY" -eq 1 ] || { echo "Local manifest server failed to start" >&2; exit 1; }
if curl --max-time 1 -fsS "http://127.0.0.1:$PORT/AGENTS.md" >/dev/null 2>&1; then
  echo 'Local manifest server exposed a repository-root file' >&2
  exit 1
fi
export PW_MANIFEST_PORT="$PORT"
node tests/contracts/manifest-browser-proof.cjs
