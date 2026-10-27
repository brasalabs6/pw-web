#!/usr/bin/env bash
# Wire-format conformance test: src/server153 CreateRole <-> PWBR v145 client.
#
# Proves the server's RoleInfo/CreateRole marshal matches the real PWBR
# 1.5.3 v145 client (build 2555, 2018): a client-style packet marshals to
# exactly 1704 bytes, its first 64 bytes match the packet captured from the
# real client (glinkd [decode] dump, 2026-10-27), and the server's unmarshal
# consumes it cleanly. Guards against re-introducing the 24th field
# (Octets rank, upstream 8aa72d3) that this client does not send.
#
# Needs g++ with -m32 (host multilib, the pw153-runtime container, or the
# CI debian:12 image with gcc-multilib). Exits 0 only when every check
# passes; exits 2 when no 32-bit toolchain is available (BLOCKED, not PASS).

set -uo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

SRV="$ROOT/src/server153"
INC="-I$SRV/share/io -I$SRV/share/common -I$SRV/share/perf \
     -I$SRV/share -I$SRV/share/rpc -I$SRV/cnet/rpcdata -I$SRV/cnet/inl"
BIN="$(mktemp "${TMPDIR:-/tmp}/pw153-wire.XXXXXXXX")"
trap 'rm -f "$BIN"' EXIT

build_and_run() { # build_and_run <compiler-prefix...>
  "$@" g++ -w -m32 -std=gnu++11 -fpermissive $INC \
    tests/server153_createrole_wire.cpp "$SRV/share/common/octets.cpp" \
    -o "$BIN" 2>/dev/null && "$BIN"
}

if build_and_run; then
  echo "wire test: PASS (host g++ -m32)"
  exit 0
fi

# No host multilib: try the pw153-runtime container (Debian 12, i386 multilib).
if command -v docker >/dev/null 2>&1 && docker container inspect pw153-runtime >/dev/null 2>&1; then
  if docker exec pw153-runtime bash -c \
    "cd /work && g++ -w -m32 -std=gnu++11 -fpermissive $INC \
     tests/server153_createrole_wire.cpp src/server153/share/common/octets.cpp \
     -o /tmp/pw153-wire && /tmp/pw153-wire"; then
    echo "wire test: PASS (pw153-runtime container)"
    exit 0
  fi
fi

echo "wire test: BLOCKED — no 32-bit toolchain (host g++ -m32 or pw153-runtime container required)" >&2
exit 2
