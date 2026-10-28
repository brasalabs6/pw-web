#!/usr/bin/env bash
# Prepare and start the game server (gs) against the real 1.5.3 v145 data.
#
# The gs needs the server data package (see docs/SERVER153-REPORT.md):
#   - PKGGAMED: the package's gamed directory (conf + server-only data:
#     .sev, npcgen, movemap, maps). Default /home/pwserver/gamed.
#   - GAMEDATA: the installed client's element/data (shared files that must
#     be byte-identical to the client's: elements/tasks/dyn_tasks/gshop...).
#     Default /gamedata (container mount).
# Our freshly built gs binary + libtask.so/libskill.so are copied into the
# package dir (gs-ours), the shared data files are linked from the client,
# and the package's 196-byte GSHOP_ITEM mall files are converted to the
# 220-byte layout this source expects (buy_times_limit fields).
set -uo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$REPO_ROOT/src/server153"
PKGGAMED="${PW153_PKGGAMED:-/home/pwserver/gamed}"
GAMEDATA="${PW153_GAMEDATA:-/gamedata}"
LOGS="${PW153_LOGS:-$REPO_ROOT/dist/server153/runtime/logs}"
STOP=0

while [ $# -gt 0 ]; do
  case "$1" in
    --stop) STOP=1; shift ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
done

if [ "$STOP" -eq 1 ]; then
  for p in /proc/[0-9]*; do
    [ "$(cat "$p/comm" 2>/dev/null)" = gs-ours ] && kill "$(basename "$p")" 2>/dev/null
  done
  exit 0
fi

[ -d "$PKGGAMED" ] || { echo "gs: package gamed dir not found: $PKGGAMED" >&2; exit 1; }
mkdir -p "$LOGS"

# 1) our binary + libs into the package dir
cp -f "$SRC/cgame/gs/gs" "$PKGGAMED/gs-ours"
cp -f "$SRC/cgame/gs/libtask.so" "$SRC/cgame/gs/libskill.so" "$PKGGAMED/"

# 2) shared data files from the client (must match the client byte for byte)
for f in elements.data tasks.data dyn_tasks.data aipolicy.data \
         task_npc.data gshop.data gshop1.data gshop2.data \
         domain.data domain2.data domain2_cross.data; do
  [ -f "$GAMEDATA/$f" ] && ln -sf "$GAMEDATA/$f" "$PKGGAMED/config/$f"
done

# 3) mall files: package layout is 196-byte GSHOP_ITEM (no min_vip_level in
#    buy[], no buy_times_limit/mode); this source expects 220 bytes.
# python3 guard: the runtime container may lack python3; the conversion is
# only needed on first staging (the .old backup marks it done).
if command -v python3 >/dev/null 2>&1; then
python3 - "$PKGGAMED/config" <<'EOF'
import struct, sys, os
def convert(path, out):
    data = open(path, 'rb').read()
    ts, count = struct.unpack_from('<ii', data, 0)
    body = data[8:]
    old, new = 196, 220
    if len(body) == count * new:
        return  # already converted
    if len(body) != count * old:
        print(f'gs: unexpected mall layout in {path}', file=sys.stderr)
        sys.exit(1)
    buf = bytearray(struct.pack('<ii', ts, count))
    for i in range(count):
        it = body[i*old:(i+1)*old]
        head = it[0:20]
        buys = b''.join(it[20+j*32:20+(j+1)*32] + b'\x00'*4 for j in range(4))
        gift = it[148:164]
        owners = it[164:196]
        buf += head + buys + gift + owners + b'\x00'*8
    open(out, 'wb').write(buf)
    print(f'gs: converted {os.path.basename(path)} ({count} items, {old}->{new} bytes)')
base = sys.argv[1]
for name in ('gshopsev.data', 'gshopsev1.data'):
    p, o = os.path.join(base, name), os.path.join(base, name + '.old')
    if not os.path.exists(o):
        os.rename(p, o)          # first run: keep the original as .old
    convert(o, p)
EOF
else
  for name in gshopsev.data gshopsev1.data; do
    [ -f "$PKGGAMED/config/$name.old" ] || echo "gs: WARNING: python3 unavailable and $name.old backup missing — mall conversion not verified" >&2
  done
fi

# 4) start (reference procedure from the package's start script)
( cd "$PKGGAMED" && LD_LIBRARY_PATH="$PKGGAMED" \
    nohup ./gs-ours gs01 gs.conf gmserver.conf gsalias.conf is61 \
    > "$LOGS/gs.log" 2>&1 & )
echo "gs started from $PKGGAMED (gs-ours)"
