#!/bin/bash
# Container entrypoint: boot the PW 1.5.3 server stack (8 daemons + gs with
# the world/instance maps), then serve the health endpoint (the
# Cloudflare-managed URL proxies HTTP to this port).
#
# pw-web note: the health server is busybox httpd (static binary, instant
# startup). A python3 http.server entrypoint is consistently refused by the
# platform ("no container instance can be provided" — bisected with probe
# apps: busybox-based entrypoints provision at any image size, python3-based
# ones never do), while busybox provisioned on the first request. The
# dynamic daemon+gs count is served by a CGI script (busybox httpd executes
# /cgi-bin/*); the glinkd TCP probe is done by the Worker through the
# mediation layer (getTcpPort(29000).connect()), which is a stronger check
# than a localhost connect inside the container.
#
# GS_INSTANCES (env, default "is61"): worlds the gs boots. "is61" = the
# parallel world where new characters spawn (~650MB RSS); "gs01 is61" adds
# the main world (~1.9GB RSS total — needs a bigger container instance,
# see cloudflare/worker.js).
set -u
SRV=/opt/server153
GAMED=/opt/gamed
export USER=builder

RUN=/opt/runtime
mkdir -p "$RUN/logs" /opt/state/logs /www/cgi-bin

# Health CGI: count the 8 daemons AND the gs (world server) alive in this
# container. A zombie does not count (the gs parent exits with a cosmetic
# SIGFPE after forking instances — its children are the real servers).
cat > /www/cgi-bin/health.sh << 'CGI'
#!/bin/sh
n=0
for d in gauthd gdeliveryd gamedbd uniquenamed gfactiond gacd logservice glinkd; do
  for p in /proc/[0-9]*/comm; do
    read c < "$p" 2>/dev/null
    if [ "$c" = "$d" ]; then n=$((n+1)); break; fi
  done
done
gs=0
for p in /proc/[0-9]*/comm; do
  read c < "$p" 2>/dev/null
  if [ "$c" = "gs-ours" ]; then
    st=$(grep -m1 '^State:' "${p%/comm}/status" 2>/dev/null | awk '{print $2}')
    if [ "$st" != "Z" ]; then gs=$((gs+1)); fi
  fi
done
ok=false
[ "$n" = "8" ] && [ "$gs" -ge 1 ] && ok=true
echo "Content-Type: application/json"
echo ""
echo "{\"ok\":$ok,\"daemons_alive\":$n,\"gs_alive\":$gs,\"expected_daemons\":8,\"expected_gs_min\":1}"
CGI
chmod +x /www/cgi-bin/health.sh

# Runtime confs: same localhost fixes as scripts/boot-server153.sh.
declare -A BASE=( [gauthd]=gauthd.conf [gdeliveryd]=gamesys.conf [gamedbd]=gamesys.conf
                  [uniquenamed]=gamesys.conf [gfaction]=gamesys.conf [gacd]=gamesys.conf
                  [logservice]=logservice.conf [glinkd]=gamesys.conf )
for d in "${!BASE[@]}"; do
  cp "$SRV/cnet/$d/${BASE[$d]}" "$RUN/$d.conf"
done
sed -i -E 's/address[[:space:]]*=[[:space:]]*172\.[0-9.]+/address\t\t=\t127.0.0.1/' "$RUN"/*.conf
sed -i -E '/^\[GAuthClient\]/,/^\[/ s/port[[:space:]]*=[[:space:]]*29200/port\t\t=\t9200/
           /^\[GAntiCheatClient\]/,/^\[/ s/port[[:space:]]*=[[:space:]]*29709/port\t\t=\t29702/' "$RUN/gdeliveryd.conf"
sed -i -E '/^\[GAntiCheatClient\]/,/^\[/ s/port[[:space:]]*=[[:space:]]*29701/port\t\t=\t29702/
           /^\[GLinkServer1\]/,/^\[/ s/port[[:space:]]*=[[:space:]]*9001/port\t\t=\t29000/' "$RUN/glinkd.conf"
# glinkd handshake: the real PWBR 1.5.3 v145 client (GAME_VERSION 0x010503)
# rejects any lower version with the "servidor suspenso" dialog, and requires
# the exact edition string it computes from its own data files. The committed
# gamesys.conf carries version=10204 and no edition — pin both (same fix as
# scripts/boot-server153.sh:88-91; without this the deployed container
# refuses every real client).
sed -i -E '/^\[GLinkServer1\]/,/^\[/ s/^version[[:space:]]*=.*/version\t\t=\t10503/' "$RUN/glinkd.conf"
sed -i '/^\[GLinkServer1\]/a edition\t\t=\t300000917d576784405750f171575122b6' "$RUN/glinkd.conf"
sed -i -E 's/table_charset[[:space:]]*=[[:space:]]*UCS2/table_charset\t\t=\tGBK/' "$RUN/gdeliveryd.conf" "$RUN/gfaction.conf"
# [ThreadPool]: one priority-101 worker starves the auth RPC queue when
# blocked (userlogin2/matrixpasswd2 dispatch at 101).
sed -i -E 's|^threads[[:space:]]*=.*|threads\t\t=\t(1,3)(100,1)(101,4)(0,1)|' "$RUN/gdeliveryd.conf"
sed -i '/^\[GameDBServer\]/a accepted_zone_list\t=\t1\ncross_zone_pos\t\t=\t-476:337:609,548:337:609,-476:337:-415,548:337:-415' "$RUN/gamedbd.conf"
sed -i -E 's|^tables[[:space:]]*=.*|tables\t\t=\tauction,base,city,clsconfig,config,crslogicuid,equipment,factionfortress,factioninfo,factionname,factionrelation,force,friendext,friends,globalcontrol,gtask,inventory,kingelection,mailbox,mappassword,messages,mndomainbonus,mndomaininfo,mnfactionapplyinfo,mnfactioninfo,order,playerprofile,playershop,recalluser,rolename,rolenamehis,sellpoint,serverdata,shoplog,solochallengerank,status,storehouse,syslog,task,translog,uniquedata,user,userfaction,userstore,waitdel,weborderitem,webtrade,webtradesold|' "$RUN/gamedbd.conf"
cat >> "$RUN/gamedbd.conf" << 'EOF'

[CASHVIP]
cash_vip_upgrade_score		=	(30,150,300,750,1800,4500)
cash_vip_reduce_score		=	(0,1,5,10,25,60,150)
EOF
sed -i -E '/^\[GAuthServer\]/,/^\[/{
  s/^iseckey[[:space:]]*=.*/iseckey\t\t=\trdppjtaki1MxoHnsnaltiiwfjszs9l/;
  s/^oseckey[[:space:]]*=.*/oseckey\t\t=\tn1hxpxztozyxnsvk6RaycpmrCnrdds/;
  s/^shared_key[[:space:]]*=.*/shared_key\t\t=\t4khdwAAcjrg0eqfzazqcemdpgulnje/
}' "$RUN/gauthd.conf"

# Storage dirs (fresh; the WDB engine has no CORRUPT-state recovery).
for d in gauthd gamedbd uniquenamed gfaction; do
  mkdir -p "$SRV/cnet/$d"/{dbhome,dbdata,dblogs,backup}
done
mkdir -p "$SRV/cnet/gamedbd"/{dbhomewdb,backupwdb} "$SRV/cnet/uniquenamed"/{dbhomewdb,unamewdb,backupwdb}
mkdir -p "$SRV/cnet/logservice/logs"

start() { # start <dir> <bin> [args...]
  local dir="$1" bin="$2"; shift 2
  ( cd "$dir" && nohup "./$bin" "$@" >> /opt/state/logs/$(basename "$dir").log 2>&1 & )
}

start "$SRV/cnet/gauthd"      gauthd      "$RUN/gauthd.conf"
start "$SRV/cnet/gdeliveryd"  gdeliveryd  "$RUN/gdeliveryd.conf"
start "$SRV/cnet/gamedbd"     gamedbd      "$RUN/gamedbd.conf"
start "$SRV/cnet/uniquenamed" uniquenamed "$RUN/uniquenamed.conf"
start "$SRV/cnet/gfaction"    gfactiond   "$RUN/gfaction.conf"
start "$SRV/cnet/gacd"        gacd        "$RUN/gacd.conf"
start "$SRV/cnet/logservice"  logservice  "$RUN/logservice.conf"
start "$SRV/cnet/glinkd"      glinkd      "$RUN/glinkd.conf" 1

# gs: the world/instance server (the maps). It registers as a provider with
# gdeliveryd:29300 and without it no world exists and logins never complete.
# Wait for the provider port before starting, then keep it alive (the gs has
# no other supervisor inside the container).
GS_INSTANCES="${GS_INSTANCES:-is61}"
if [ -x "$GAMED/gs-ours" ]; then
  # Map data ships in /opt/gamed/maps (separate image layers); link each map
  # into config/ where the gs expects it (config/a61, config/world, ...).
  for m in "$GAMED"/maps/*; do
    [ -e "$m" ] && ln -sfn "$m" "$GAMED/config/$(basename "$m")"
  done
  wait_port() { # wait_port <port> [tries]
    local p="$1" tries="${2:-90}" i
    for i in $(seq 1 "$tries"); do
      (exec 3<>"/dev/tcp/127.0.0.1/$p") 2>/dev/null && { exec 3>&- 3<&-; return 0; }
      sleep 2
    done
    return 1
  }
  if ! wait_port 29300 45; then
    echo "$(date -u +%FT%TZ) entrypoint: gdeliveryd:29300 not listening; starting gs anyway" >> /opt/state/logs/watchdog.log
  fi
  read -ra INST <<< "$GS_INSTANCES"
  gs_start() {
    ( cd "$GAMED" && LD_LIBRARY_PATH="$GAMED" \
      nohup ./gs-ours "${INST[0]}" gs.conf gmserver.conf gsalias.conf "${INST[@]:1}" \
      >> /opt/state/logs/gs.log 2>&1 & )
  }
  gs_start
  # mini-watchdog: restart the gs if every gs-ours process is gone (zombies
  # do not count — the fork parent exits with a cosmetic SIGFPE).
  ( while true; do
      sleep 30
      alive=0
      for p in /proc/[0-9]*/comm; do
        [ "$(cat "$p" 2>/dev/null)" = "gs-ours" ] || continue
        st=$(grep -m1 '^State:' "${p%/comm}/status" 2>/dev/null | awk '{print $2}')
        [ "$st" != "Z" ] && { alive=1; break; }
      done
      if [ "$alive" != "1" ]; then
        echo "$(date -u +%FT%TZ) entrypoint: gs down, restarting" >> /opt/state/logs/watchdog.log
        gs_start
      fi
    done ) &
else
  echo "$(date -u +%FT%TZ) entrypoint: gs binary missing — no world will come up" >> /opt/state/logs/watchdog.log
fi

# Log rotation: the daemons append forever; cap each log at ~10MB using the
# copytruncate pattern (tail the last 10MB back into the same inode so the
# daemons' open append fds keep working).
( while true; do
    sleep 1800
    for f in /opt/state/logs/*.log; do
      [ -f "$f" ] || continue
      sz=$(stat -c %s "$f" 2>/dev/null) || continue
      if [ "${sz:-0}" -gt 20971520 ]; then
        tail -c 10485760 "$f" > "$f.rt" 2>/dev/null && cat "$f.rt" > "$f" && rm -f "$f.rt"
      fi
    done
  done ) &

exec busybox httpd -f -p 8080 -h /www
