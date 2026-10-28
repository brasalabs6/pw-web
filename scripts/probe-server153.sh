#!/usr/bin/env bash
# Handshake probe: run the in-tree virtual client (gclient) against a running
# server and assert the GNET protocol answers.
#
# Usage: scripts/probe-server153.sh [--src DIR] [--out DIR] [--wait N]
#
# Success criteria (all must hold):
#   1. gclient establishes GNET sessions (OnAddSession lines > 0).
#   2. glinkd receives and processes client protocols (matrixpasswd traces).
#   3. The expected port (29000) is listening before the probe.
set -uo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$REPO_ROOT/src/server153"
OUT="$REPO_ROOT/dist/server153"
WAIT=12

while [ $# -gt 0 ]; do
  case "$1" in
    --src) SRC="${2:?}"; shift 2 ;;
    --out) OUT="${2:?}"; shift 2 ;;
    --wait) WAIT="${2:?}"; shift 2 ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
done

RUN="$OUT/runtime"
LOGS="$RUN/logs"
mkdir -p "$OUT/probe"

PORT_HEX=$(printf '%04X' 29000)
if ! grep -q ":$PORT_HEX" /proc/net/tcp 2>/dev/null; then
  echo "FAIL: port 29000 (0x$PORT_HEX) is not listening" >&2
  exit 1
fi

echo "=== gclient probe against 127.0.0.1:29000 (${WAIT}s) ==="
( cd "$SRC/cnet/gclient" && timeout "$WAIT" ./gclient "$RUN/gclient.conf" ) \
  > "$OUT/probe/gclient.log" 2>&1

SESSIONS=$(grep -c 'OnAddSession' "$OUT/probe/gclient.log" || true)
echo "gclient sessions established: $SESSIONS"
if [ "${SESSIONS:-0}" -lt 1 ]; then
  echo "FAIL: no GNET session established" >&2
  exit 1
fi

# Server-side evidence: glinkd must have processed client protocols.
GCLIENT_SIDE=$(grep -cE 'receive response from client|receive matrixpasswd' "$LOGS/glinkd.log" || true)
echo "glinkd processed client protocols: $GCLIENT_SIDE (cumulative)"
if [ "${GCLIENT_SIDE:-0}" -lt 1 ]; then
  echo "FAIL: glinkd shows no client protocol processing" >&2
  exit 1
fi

echo "PROBE OK — GNET handshake answered on 127.0.0.1:29000"
exit 0
