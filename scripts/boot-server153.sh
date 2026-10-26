#!/usr/bin/env bash
# Boot the built PW 1.5.3 server (src/server153) locally and prove it runs.
#
# Generates runtime config copies in dist/server153/runtime/ with documented
# localhost fixes (the committed gamesys.conf files come from different
# upstream vintages and disagree on a few cross-daemon ports), then starts:
#   gauthd -> gdeliveryd -> gamedbd -> uniquenamed -> gfaction -> gacd
#   -> logservice -> glinkd -> gs
# and reports process/port status. gs requires game data files (elements.data,
# ...) that are NOT part of this source; its failure is expected and captured.
#
# Usage: scripts/boot-server153.sh [--src DIR] [--out DIR] [--stop] [--wait N]
set -uo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$REPO_ROOT/src/server153"
OUT="$REPO_ROOT/dist/server153"
WAIT=20
STOP=0
FRESH=0

while [ $# -gt 0 ]; do
  case "$1" in
    --src)  SRC="${2:?}"; shift 2 ;;
    --out)  OUT="${2:?}"; shift 2 ;;
    --wait) WAIT="${2:?}"; shift 2 ;;
    --stop) STOP=1; shift ;;
    --fresh) FRESH=1; shift ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
done

RUN="$OUT/runtime"
LOGS="$RUN/logs"
mkdir -p "$RUN" "$LOGS"

DAEMONS=(gauthd gdeliveryd gamedbd uniquenamed gfaction gacd logservice glinkd gs)

stop_all() {
  local d p
  for d in "${DAEMONS[@]}"; do
    for p in /proc/[0-9]*; do
      if [ "$(cat "$p/comm" 2>/dev/null)" = "${BIN[$d]}" ]; then
        kill "$(basename "$p")" 2>/dev/null && echo "stopped ${BIN[$d]}" || true
      fi
    done
  done
  sleep 1
}

if [ "$STOP" -eq 1 ]; then
  stop_all
  exit 0
fi

# ---------------------------------------------------------------- conf fixes
# Base conf per daemon: gamesys.conf where the upstream "begin" flow uses it,
# dedicated conf otherwise. Runtime copies get localhost fixes only.
declare -A BASE=( [gauthd]=gauthd.conf [gdeliveryd]=gamesys.conf [gamedbd]=gamesys.conf
                  [uniquenamed]=gamesys.conf [gfaction]=gamesys.conf [gacd]=gamesys.conf
                  [logservice]=logservice.conf [glinkd]=gamesys.conf )
declare -A BIN=( [gauthd]=gauthd [gdeliveryd]=gdeliveryd [gamedbd]=gamedbd
                 [uniquenamed]=uniquenamed [gfaction]=gfactiond [gacd]=gacd
                 [logservice]=logservice [glinkd]=glinkd [gs]=gs )

for d in "${!BASE[@]}"; do
  cp "$SRC/cnet/$d/${BASE[$d]}" "$RUN/$d.conf"
done

# All upstream cross-host addresses (172.16.x.x) become localhost.
sed -i -E 's/address[[:space:]]*=[[:space:]]*172\.[0-9.]+/address\t\t=\t127.0.0.1/' "$RUN"/*.conf

# gdeliveryd: gauthd listens on 9200 (gauthd.conf), not the 29200 in this
# gamesys.conf vintage; gacd listens on 29702 (gacd gamesys.conf), not 29709.
sed -i -E '/^\[GAuthClient\]/,/^\[/ s/port[[:space:]]*=[[:space:]]*29200/port\t\t=\t9200/
           /^\[GAntiCheatClient\]/,/^\[/ s/port[[:space:]]*=[[:space:]]*29709/port\t\t=\t29702/' "$RUN/gdeliveryd.conf"
# glinkd: gacd port 29701 -> 29702 for the same reason, and GLinkServer1
# port 9001 -> 29000 (classic PW client port; the real client's serverlist
# points at 127.0.0.1:29000). The committed gamesys.conf carries
# version = 10204, which the conf parser resolves AFTER our appended key —
# substitute the original key in place (the client, PWBR 1.5.3 v145 with
# GAME_VERSION 0x010503, rejects any lower version with the "servidor
# suspenso" dialog) and pin the edition (the client requires the exact
# string it computes from its own data files).
sed -i -E '/^\[GAntiCheatClient\]/,/^\[/ s/port[[:space:]]*=[[:space:]]*29701/port\t\t=\t29702/' "$RUN/glinkd.conf"
sed -i -E '/^\[GLinkServer1\]/,/^\[/ s/port[[:space:]]*=[[:space:]]*9001/port\t\t=\t29000/' "$RUN/glinkd.conf"
sed -i -E '/^\[GLinkServer1\]/,/^\[/ s/^version[[:space:]]*=.*/version\t\t=\t10503/' "$RUN/glinkd.conf"
sed -i '/^\[GLinkServer1\]/a edition\t\t=\t300000917d576784405750f171575122b6' "$RUN/glinkd.conf"

# gdeliveryd/gfaction: the committed `filters` sensitive-word tables are GBK
# (no BOM), but these gamesys.conf vintages declare table_charset=UCS2 — the
# conversion then produces garbage and the daemons refuse to start.
sed -i -E 's/table_charset[[:space:]]*=[[:space:]]*UCS2/table_charset\t\t=\tGBK/' "$RUN/gdeliveryd.conf" "$RUN/gfaction.conf"

# gamedbd: InitGameDB requires accepted_zone_list (single zone here) and a
# cross_zone_pos with exactly 4 "x:y:z" entries; these keys are absent from
# the committed gamesys.conf. The code also opens ~48 storages via GetStorage()
# while the committed [storagewdb] tables list names only 26 — GetStorage("syslog")
# (used by InitGUID) then returns NULL and gamedbd segfaults. Extend the runtime
# tables list to cover every storage the code may open (missing ones are created
# on demand as empty files).
sed -i '/^\[GameDBServer\]/a accepted_zone_list\t=\t1\ncross_zone_pos\t\t=\t-476:337:609,548:337:609,-476:337:-415,548:337:-415' "$RUN/gamedbd.conf"
sed -i -E 's|^tables[[:space:]]*=.*|tables\t\t=\tauction,base,city,clsconfig,config,crslogicuid,equipment,factionfortress,factioninfo,factionname,factionrelation,force,friendext,friends,globalcontrol,gtask,inventory,kingelection,mailbox,mappassword,messages,mndomainbonus,mndomaininfo,mnfactionapplyinfo,mnfactioninfo,order,playerprofile,playershop,recalluser,rolename,rolenamehis,sellpoint,serverdata,shoplog,solochallengerank,status,storehouse,syslog,task,translog,uniquedata,user,userfaction,userstore,waitdel,weborderitem,webtrade,webtradesold|' "$RUN/gamedbd.conf"

# gamedbd: [CASHVIP] section (cash VIP score thresholds; canonical values from
# the commented defaults in cnet/gamedbd/utilfunction.h) — absent from the
# committed gamesys.conf and required by InitGameDB.
cat >> "$RUN/gamedbd.conf" <<'EOF'

[CASHVIP]
cash_vip_upgrade_score		=	(30,150,300,750,1800,4500)
cash_vip_reduce_score		=	(0,1,5,10,25,60,150)
EOF

# gauthd: the committed gauthd.conf carries placeholder keys (456/123/123456)
# while gdeliveryd's gamesys.conf carries real random keys, so the
# delivery->authd stream decrypted to garbage ("Protocol Unknown type=3261"
# and a reconnect loop). Cross-match gauthd's runtime keys to gdeliveryd's:
# delivery.oseckey -> gauthd.iseckey, delivery.iseckey -> gauthd.oseckey.
sed -i -E '/^\[GAuthServer\]/,/^\[/{
  s/^iseckey[[:space:]]*=.*/iseckey\t\t=\trdppjtaki1MxoHnsnaltiiwfjszs9l/;
  s/^oseckey[[:space:]]*=.*/oseckey\t\t=\tn1hxpxztozyxnsvk6RaycpmrCnrdds/;
  s/^shared_key[[:space:]]*=.*/shared_key\t\t=\t4khdwAAcjrg0eqfzazqcemdpgulnje/
}' "$RUN/gauthd.conf"

# --fresh: wipe daemon storage state (validation only; the databases are
# empty at this stage and the WDB engine has no CORRUPT-state recovery, so
# state left by a hard kill can permanently break gamedbd startup).
if [ "$FRESH" -eq 1 ]; then
  for d in gauthd gamedbd uniquenamed gfaction; do
    rm -rf "$SRC/cnet/$d"/{dbhome,dbdata,dblogs,backup,dbhomewdb,backupwdb}
  done
fi

# Probe client conf: glinkd listens on 29000; the dev-auth requires numeric
# accounts (multiples of 16), so pin the probe account range accordingly.
sed -E 's/address[[:space:]]*=[[:space:]]*[0-9.]+/address\t\t\t=\t127.0.0.1/;
        s/port[[:space:]]*=[[:space:]]*29000/port\t\t\t=\t29000/' \
  "$SRC/cnet/gclient/gclient.conf" > "$RUN/gclient.conf"
sed -i -E 's/^prefix.*/prefix\t\t\t=\t/; s/^start.*/start\t\t\t=\t1000000/; s/^end.*/end\t\t\t\t=\t1000001/' "$RUN/gclient.conf"

# gs game data: link the v145 data files from the installed real client
# (see docs/RESEARCH.md, "Assets do cliente 1.5.3"). gs.conf [Template]
# expects them under ./config/ relative to the gs cwd.
GAMEDATA="${PW153_GAMEDATA:-/home/nonexphere/workspace/pw/game/client/element/data}"
if [ -d "$GAMEDATA" ]; then
  mkdir -p "$SRC/cgame/gs/config"
  for f in elements.data tasks.data dyn_tasks.data aipolicy.data \
           task_npc.data gshop.data gshop1.data gshop2.data \
           domain.data domain2.data domain2_cross.data; do
    [ -f "$GAMEDATA/$f" ] && ln -sf "$GAMEDATA/$f" "$SRC/cgame/gs/config/$f"
  done
fi

# ------------------------------------------------------------- storage dirs
for d in gauthd gamedbd uniquenamed gfaction; do
  mkdir -p "$SRC/cnet/$d"/{dbhome,dbdata,dblogs,backup}
done
mkdir -p "$SRC/cnet/gamedbd"/{dbhomewdb,backupwdb} "$SRC/cnet/uniquenamed"/{dbhomewdb,backupwdb}
mkdir -p "$SRC/cnet/logservice/logs"

# -------------------------------------------------------------------- boot
start() { # start <dir> <bin> [args...]
  local dir="$1" bin="$2"; shift 2
  local name="${dir##*/}"
  # Only gs needs LD_LIBRARY_PATH (libtask.so/libskill.so live in its dir);
  # other daemons must NOT have it (gacd would load its stale committed libxml2.so.2).
  if [ "$name" = gs ]; then
    ( cd "$dir" && LD_LIBRARY_PATH="$dir" nohup "./$bin" "$@" >"$LOGS/$name.log" 2>&1 & )
  else
    ( cd "$dir" && nohup "./$bin" "$@" >"$LOGS/$name.log" 2>&1 & )
  fi
  echo "started $name"
}

alive() { # alive <comm> — /proc-based, works without procps
  local p
  for p in /proc/[0-9]*; do
    [ "$(cat "$p/comm" 2>/dev/null)" = "$1" ] && return 0
  done
  return 1
}

listen_ports() { # decimal ports from /proc/net/tcp and /proc/net/udp (portable awk)
  awk 'NR>1 { n=split($2,a,":"); h=a[2]; d=0
    for (i=1; i<=length(h); i++) { c=tolower(substr(h,i,1))
      d = d*16 + (c < "a" ? c+0 : index("abcdef",c)+9) }
    print d }' /proc/net/tcp /proc/net/udp 2>/dev/null | sort -un
}

echo "=== booting server153 (order: ${DAEMONS[*]}) ==="
start "$SRC/cnet/gauthd"      gauthd      "$RUN/gauthd.conf"
start "$SRC/cnet/gdeliveryd"  gdeliveryd  "$RUN/gdeliveryd.conf"
start "$SRC/cnet/gamedbd"     gamedbd     "$RUN/gamedbd.conf"
start "$SRC/cnet/uniquenamed" uniquenamed "$RUN/uniquenamed.conf"
start "$SRC/cnet/gfaction"    gfactiond   "$RUN/gfaction.conf"
start "$SRC/cnet/gacd"        gacd        "$RUN/gacd.conf"
start "$SRC/cnet/logservice"  logservice  "$RUN/logservice.conf"
# glinkd: instance argument "1" selects the [GLinkServer1] section.
start "$SRC/cnet/glinkd"      glinkd      "$RUN/glinkd.conf" 1

# gs: run from the real 1.5.3 v145 server data package when available
# (scripts/gs-server153.sh prepares the data and starts our binary);
# otherwise start from the source tree (will fail without game data).
if [ -x "$REPO_ROOT/scripts/gs-server153.sh" ] && [ -d "${PW153_PKGGAMED:-/home/pwserver/gamed}" ]; then
  "$REPO_ROOT/scripts/gs-server153.sh"
else
  start "$SRC/cgame/gs" gs
fi

echo "=== waiting ${WAIT}s for stabilization ==="
sleep "$WAIT"

echo
echo "=== processes ==="
for d in "${DAEMONS[@]}"; do
  if alive "${BIN[$d]}"; then
    echo "ALIVE   ${BIN[$d]}"
  else
    echo "DEAD    ${BIN[$d]}"
  fi
done

echo
echo "=== listening sockets (expect 9200 gauthd, 29100 gdeliveryd, 29300 gprovider,"
echo "    29400 gamedbd, 29401 uniquenamed, 29500 gfaction, 29702/29712 gacd,"
echo "    11100/11101 logservice, 29000 glinkd) ==="
listen_ports | grep -E '^(9200|29100|29300|29400|29401|29500|29702|29712|11100|11101|29000)$' || echo "no expected sockets found"

echo
echo "logs: $LOGS"
