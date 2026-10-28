#!/usr/bin/env bash
# Stage and validate the Cloudflare Containers image for the PW 1.5.3 server.
#
# Usage:
#   scripts/cloudflare-server153.sh stage    — build the staging context
#   scripts/cloudflare-server153.sh local    — build the image and test it
#                                              locally (health + probe)
#   scripts/cloudflare-server153.sh deploy   — wrangler containers build + deploy
#
# The staging context is dist/server153/cf-context/: the built daemon tree
# (binaries stripped, objects removed) plus the cloudflare/ entry files.
set -uo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$REPO_ROOT/src/server153"
OUT="$REPO_ROOT/dist/server153"
CTX="$OUT/cf-context"
CF="$REPO_ROOT/cloudflare"
CMD="${1:-stage}"

stage() {
  rm -rf "$CTX"
  mkdir -p "$CTX/server153/cnet" "$CTX/server153/cgame"
  for d in gauthd gdeliveryd gamedbd uniquenamed gfaction gacd logservice glinkd; do
    cp -r "$SRC/cnet/$d" "$CTX/server153/cnet/$d"
  done
  cp -r "$SRC/cgame/gs" "$CTX/server153/cgame/gs"
  # Drop build artifacts; keep binaries, confs, xml and runtime data files.
  find "$CTX" \( -name '*.o' -o -name '*.a' -o -name '*.so' -o -name '*.hpp' \
        -o -name '*.cpp' -o -name '*.cxx' -o -name '*.h' -o -name '*.hrp' \
        -o -name 'Makefile*' -o -name '*.bak' \) -delete
  # Drop ALL daemon storage state: a partial WDB copy (root-owned files fail
  # mid-cp) breaks storage init ("Initialize storage environment failed").
  rm -rf "$CTX"/server153/cnet/*/{dbhome,dbdata,dblogs,backup,dbhomewdb,unamewdb,backupwdb}
  cp "$CF/Dockerfile" "$CF/entrypoint.sh" "$CF/health-server.py" "$CTX/"
  du -sh "$CTX"
}

local_test() {
  stage
  docker build -q -t pw153-cf "$CTX" && echo "image built"
  docker rm -f pw153-cf-test >/dev/null 2>&1
  docker run -d --name pw153-cf-test -p 127.0.0.1:18080:8080 pw153-cf >/dev/null
  echo "waiting 40s for daemons..."
  sleep 40
  echo "=== /health ==="
  curl -s -m 10 http://127.0.0.1:18080/health; echo
  echo "=== /probe ==="
  curl -s -m 10 http://127.0.0.1:18080/probe; echo
  echo "=== daemon logs (tails) ==="
  docker exec pw153-cf-test bash -c 'for f in /opt/state/logs/*.log; do echo "-- $(basename $f)"; tail -2 "$f"; done' | head -30
  docker rm -f pw153-cf-test >/dev/null
}

deploy() {
  stage
  cd "$CF"
  npx wrangler containers build "$CTX" --tag pw-server153
  npx wrangler deploy
}

case "$CMD" in
  stage)  stage ;;
  local)  local_test ;;
  deploy) deploy ;;
  *) echo "usage: $0 stage|local|deploy" >&2; exit 2 ;;
esac
