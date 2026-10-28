#!/usr/bin/env bash
# Watchdog for the PW 1.5.3 server: keep all 8 daemons + gs alive.
#
# The vendored source still carries latent concurrency bugs (each found and
# fixed structurally — see docs/SERVER153-REPORT.md §4). Until the remaining
# ones are hunted with sanitizer builds, this watchdog converts a crash
# into a seconds-long blip: it detects a dead daemon and restarts it in the
# documented order (dependencies first), and restarts the gs from the
# package procedure.
#
# Usage: scripts/watchdog-server153.sh [--interval N]   (default 10s)
# Runs forever; kill it to stop. Safe to run multiple times (it is
# idempotent per cycle: only dead daemons are restarted).

set -u
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
INTERVAL=10
while [ $# -gt 0 ]; do
  case "$1" in
    --interval) INTERVAL="${2:?}"; shift 2 ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
done

declare -A BIN=( [gauthd]=gauthd [gdeliveryd]=gdeliveryd [gamedbd]=gamedbd
                  [uniquenamed]=uniquenamed [gfaction]=gfactiond [gacd]=gacd
                  [logservice]=logservice [glinkd]=glinkd )
declare -A BASE=( [gauthd]=gauthd.conf [gdeliveryd]=gamesys.conf [gamedbd]=gamesys.conf
                  [uniquenamed]=gamesys.conf [gfaction]=gamesys.conf [gacd]=gamesys.conf
                  [logservice]=logservice.conf [glinkd]=gamesys.conf )
# Restart order: dependencies first (matches scripts/boot-server153.sh).
ORDER=(gauthd gdeliveryd gamedbd uniquenamed gfaction gacd logservice glinkd)
SRC="$REPO_ROOT/src/server153"
RUN="$REPO_ROOT/dist/server153/runtime"
LOGS="$RUN/logs"

alive() { # alive <comm>
  local p
  for p in /proc/[0-9]*/comm; do
    [ "$(cat "$p" 2>/dev/null)" = "$1" ] && return 0
  done
  return 1
}

restart_daemon() { # restart_daemon <key>
  local d="$1"
  echo "$(date -u +%FT%TZ) watchdog: restarting $d" >> "$LOGS/watchdog.log"
  if [ "$d" = "glinkd" ]; then
    # glinkd requires the instance argument ("1" selects [GLinkServer1]).
    ( cd "$SRC/cnet/$d" && nohup "./${BIN[$d]}" "$RUN/$d.conf" 1 >> "$LOGS/$d.log" 2>&1 & )
  else
    ( cd "$SRC/cnet/$d" && nohup "./${BIN[$d]}" "$RUN/$d.conf" >> "$LOGS/$d.log" 2>&1 & )
  fi
}

echo "$(date -u +%FT%TZ) watchdog started (interval ${INTERVAL}s)"
while true; do
  sleep "$INTERVAL"
  for d in "${ORDER[@]}"; do
    alive "${BIN[$d]}" || restart_daemon "$d"
  done
  if ! alive gs-ours; then
    echo "$(date -u +%FT%TZ) watchdog: gs down, restarting via gs-server153.sh" >> "$LOGS/watchdog.log"
    "$REPO_ROOT/scripts/gs-server153.sh" >> "$LOGS/watchdog.log" 2>&1
  fi
done
