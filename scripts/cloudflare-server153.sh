#!/usr/bin/env bash
# Stage and validate the Cloudflare Containers image for the PW 1.5.3 server.
#
# Usage:
#   scripts/cloudflare-server153.sh stage    — build the staging context
#   scripts/cloudflare-server153.sh local    — build the image and test it
#                                              locally (health + probe)
#   scripts/cloudflare-server153.sh deploy   — wrangler containers build + deploy
#
# The staging context is cloudflare/ itself: `wrangler containers build` uses
# the wrangler config directory as the build context (the PATH positional is
# not honored by wrangler 4.x), so the daemon tree goes to cloudflare/server153/
# and the game data package to cloudflare/gamed/ (both gitignored).
set -uo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$REPO_ROOT/src/server153"
CF="$REPO_ROOT/cloudflare"
CMD="${1:-stage}"

stage() {
  # The local runtime container writes daemon storage as root (bind mount);
  # those root-owned log files abort cp mid-tree with Permission denied.
  # They are dropped from staging anyway — make the tree readable first
  # (best effort: skip silently when sudo is unavailable).
  sudo -n chown -R nonexphere:nonexphere "$SRC/cnet" 2>/dev/null || true
  rm -rf "$CF/server153" "$CF/gamed"
  mkdir -p "$CF/server153/cnet" "$CF/server153/cgame" "$CF/gamed/config"
  for d in gauthd gdeliveryd gamedbd uniquenamed gfaction gacd logservice glinkd; do
    cp -r "$SRC/cnet/$d" "$CF/server153/cnet/$d"
  done
  cp -r "$SRC/cgame/gs" "$CF/server153/cgame/gs"
  # Drop build artifacts; keep binaries, confs, xml and runtime data files.
  find "$CF/server153" \( -name '*.o' -o -name '*.a' -o -name '*.so' -o -name '*.hpp' \
        -o -name '*.cpp' -o -name '*.cxx' -o -name '*.h' -o -name '*.hrp' \
        -o -name 'Makefile*' -o -name '*.bak' \) -delete
  # Drop ALL daemon storage state: a partial WDB copy (root-owned files fail
  # mid-cp) breaks storage init ("Initialize storage environment failed").
  rm -rf "$CF"/server153/cnet/*/{dbhome,dbdata,dblogs,backup,dbhomewdb,unamewdb,backupwdb}

  # gamed: the server data package (world maps + shared game data). The gs
  # cannot run without it — this is why no world/map ever came up in previous
  # deploys. Maps shipped: world/ (gs01, worldtag 1) and a61/ (is61,
  # worldtag 161 — where new characters spawn). The runtime boots is61 by
  # default (GS_INSTANCES in cloudflare/worker.js); gs01 needs ~2GB RAM.
  local PKGGAMED="${PW153_PKGGAMED:-/home/nonexphere/workspace/pw/clients/pwserver_153v145/home/pwserver/gamed}"
  local GAMEDATA="${PW153_GAMEDATA:-/home/nonexphere/workspace/pw/game/client/element/data}"
  [ -d "$PKGGAMED/config" ] || { echo "stage: package gamed not found: $PKGGAMED" >&2; exit 1; }
  cp "$PKGGAMED"/gs.conf "$PKGGAMED"/gmserver.conf "$PKGGAMED"/gsalias.conf \
     "$PKGGAMED"/ptemplate.conf "$CF/gamed/"
  # gs.conf AND gsalias.conf ship the package's absolute install path as
  # [Template] Root (/home/pwserver/gamed/config/) — it only works in the
  # local container because the bind mount matches. The alias file is
  # APPENDED to the same Conf object at boot (FirstStepInit), so its
  # [Template] section wins. Normalize both to the cwd so the gs finds the
  # data at /opt/gamed in the image.
  sed -i -E 's|^Root[[:space:]]*=.*|Root\t\t=\t./config/|' \
    "$CF/gamed/gs.conf" "$CF/gamed/gsalias.conf"
  # config/ root files (wmgj.nmd, *.sev, filters, gshopsev, lua...) — no map dirs
  find "$PKGGAMED/config" -maxdepth 1 -type f -exec cp '{}' "$CF/gamed/config/" \;
  # Map data, as separate image layers (one large COPY layer failed the
  # registry push with 401 during the upload). Default: a61 only (is61, the
  # parallel world where new characters spawn — the full playable flow).
  # PW153_CF_MAPS="world a61" ships the main world too (~2GB RAM instance).
  local MAPS="${PW153_CF_MAPS:-a61}"
  mkdir -p "$CF/gamed/maps"
  read -ra M <<< "$MAPS"
  for m in "${M[@]}"; do
    [ -d "$PKGGAMED/config/$m" ] && cp -r "$PKGGAMED/config/$m" "$CF/gamed/maps/" \
      || { echo "stage: map not found in package: $m" >&2; exit 1; }
  done
  # client-shared data files (must be byte-identical to the real client's)
  for f in elements.data tasks.data dyn_tasks.data aipolicy.data \
           task_npc.data gshop.data gshop1.data gshop2.data \
           domain.data domain2.data domain2_cross.data; do
    [ -f "$GAMEDATA/$f" ] && cp "$GAMEDATA/$f" "$CF/gamed/config/"
  done
  # Mall files: the package layout is 196-byte GSHOP_ITEM; this source expects
  # 220. Convert in-place here (host python3) so the container needs none.
  python3 - "$CF/gamed/config" <<'PYEOF'
import struct, sys, os
def convert(path):
    data = open(path, 'rb').read()
    ts, count = struct.unpack_from('<ii', data, 0)
    body = data[8:]
    old, new = 196, 220
    if len(body) == count * new:
        return  # already converted
    if len(body) != count * old:
        print(f'gs: unexpected mall layout in {path}', file=sys.stderr); sys.exit(1)
    buf = bytearray(struct.pack('<ii', ts, count))
    for i in range(count):
        it = body[i*old:(i+1)*old]
        head = it[0:20]
        buys = b''.join(it[20+j*32:20+(j+1)*32] + b'\x00'*4 for j in range(4))
        gift = it[148:164]
        owners = it[164:196]
        buf += head + buys + gift + owners + b'\x00'*8
    open(path, 'wb').write(buf)
    print(f'gs: converted {os.path.basename(path)} ({count} items, {old}->{new} bytes)')
base = sys.argv[1]
for name in ('gshopsev.data', 'gshopsev1.data'):
    convert(os.path.join(base, name))
PYEOF
  # our gs binary + libs, stripped on the host (smaller build context)
  strip -o "$CF/gamed/gs-ours" "$SRC/cgame/gs/gs"
  cp "$SRC/cgame/gs/libtask.so" "$CF/gamed/"
  strip -o "$CF/gamed/libskill.so" "$SRC/cskill/skill/libskill.so"
  # the package carries the pre-conversion originals as *.old — not needed
  rm -f "$CF/gamed/config/"*.old
  du -sh "$CF/server153" "$CF/gamed"
}

local_test() {
  stage
  docker build -q -t pw153-cf "$CF" && echo "image built"
  docker rm -f pw153-cf-test >/dev/null 2>&1
  docker run -d --name pw153-cf-test -p 127.0.0.1:18080:8080 pw153-cf >/dev/null
  echo "waiting 150s for daemons + gs (world load)..."
  sleep 150
  echo "=== /cgi-bin/health.sh (dynamic daemon+gs count) ==="
  curl -s -m 10 http://127.0.0.1:18080/cgi-bin/health.sh; echo
  echo "=== gdeliveryd provider announce (world up) ==="
  docker exec pw153-cf-test bash -c 'grep -a "worldtag is" /opt/state/logs/gdeliveryd.log | tail -4' 2>/dev/null
  echo "=== gs log tail ==="
  docker exec pw153-cf-test bash -c 'tail -5 /opt/state/logs/gs.log' 2>/dev/null
  echo "=== daemon logs (tails) ==="
  docker exec pw153-cf-test bash -c 'for f in /opt/state/logs/*.log; do echo "-- $(basename $f)"; tail -2 "$f"; done' | head -30
  docker rm -f pw153-cf-test >/dev/null
}

deploy() {
  stage
  cd "$CF"
  npx wrangler containers build "$CF" --tag pw-server153
  npx wrangler deploy
}

case "$CMD" in
  stage)  stage ;;
  local)  local_test ;;
  deploy) deploy ;;
  *) echo "usage: $0 stage|local|deploy" >&2; exit 2 ;;
esac
