#!/usr/bin/env bash
# Container entrypoint: boot the PW 1.5.3 server, then serve the health
# endpoint (the Cloudflare-managed URL proxies HTTP to this port).
set -u
SRV=/opt/server153
export USER=builder

RUN=/opt/runtime
mkdir -p "$RUN/logs" /opt/state/logs

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
sed -i -E 's/table_charset[[:space:]]*=[[:space:]]*UCS2/table_charset\t\t=\tGBK/' "$RUN/gdeliveryd.conf" "$RUN/gfaction.conf"
sed -i '/^\[GameDBServer\]/a accepted_zone_list\t=\t1\ncross_zone_pos\t\t=\t-476:337:609,548:337:609,-476:337:-415,548:337:-415' "$RUN/gamedbd.conf"
sed -i -E 's|^tables[[:space:]]*=.*|tables\t\t=\tauction,base,city,clsconfig,config,crslogicuid,equipment,factionfortress,factioninfo,factionname,factionrelation,force,friendext,friends,globalcontrol,gtask,inventory,kingelection,mailbox,mappassword,messages,mndomainbonus,mndomaininfo,mnfactionapplyinfo,mnfactioninfo,order,playerprofile,playershop,recalluser,rolename,rolenamehis,sellpoint,serverdata,shoplog,solochallengerank,status,storehouse,syslog,task,translog,uniquedata,user,userfaction,userstore,waitdel,weborderitem,webtrade,webtradesold|' "$RUN/gamedbd.conf"
cat >> "$RUN/gamedbd.conf" <<'EOF'

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
mkdir -p "$SRV/cnet/gamedbd"/{dbhomewdb,backupwdb} "$SRV/cnet/uniquenamed"/{dbhomewdb,backupwdb}
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

# gs requires the server data package (absent); see docs/SERVER153-REPORT.md.
exec python3 /opt/health-server.py
