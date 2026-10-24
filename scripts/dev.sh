#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PORT="${PORT:-8080}"
if ! [[ "$PORT" =~ ^[0-9]+$ ]] || (( PORT < 1 || PORT > 65535 )); then
  echo "Invalid PORT: $PORT" >&2
  exit 2
fi
echo "PW Web demo: http://localhost:$PORT"
echo "This is an isolated platform demonstrator, NOT Perfect World gameplay."
exec python3 -m http.server "$PORT" --bind 127.0.0.1 --directory "$ROOT/web"
