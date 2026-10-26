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

while [ $# -gt 0 ]; do
  case "$1" in
    --src)  SRC="${2:?}"; shift 2 ;;
    --out)  OUT="${2:?}"; shift 2 ;;
    --wait) WAIT="${2:?}"; shift 2 ;;
    --stop) STOP=1; shift ;;
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
# glinkd: gacd port 29701 -> 29702 for the same reason.
sed -i -E '/^\[GAntiCheatClient\]/,/^\[/ s/port[[:space:]]*=[[:space:]]*29701/port\t\t=\t29702/' "$RUN/glinkd.conf"

# gdeliveryd/gfaction: the committed `filters` sensitive-word tables are GBK
# (no BOM), but these gamesys.conf vintages declare table_charset=UCS2 — the
# conversion then produces garbage and the daemons refuse to start.
sed -i -E 's/table_charset[[:space:]]*=[[:space:]]*UCS2/table_charset\t\t=\tGBK/' "$RUN/gdeliveryd.conf" "$RUN/gfaction.conf"

# Probe client conf: glinkd (gamesys) listens on 9001.
sed -E 's/address[[:space:]]*=[[:space:]]*[0-9.]+/address\t\t\t=\t127.0.0.1/;
        s/port[[:space:]]*=[[:space:]]*29000/port\t\t\t=\t9001/' \
  "$SRC/cnet/gclient/gclient.conf" > "$RUN/gclient.conf"

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
# gs reads gs.conf/gmserver.conf/gsalias.conf from its cwd (committed).
start "$SRC/cgame/gs"         gs

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
echo "    11100/11101 logservice, 9001 glinkd) ==="
listen_ports | grep -E '^(9200|29100|29300|29400|29401|29500|29702|29712|11100|11101|9001)$' || echo "no expected sockets found"

echo
echo "logs: $LOGS"
