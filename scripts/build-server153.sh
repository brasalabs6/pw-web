#!/usr/bin/env bash
# Build the vendored PW 1.5.3 server (src/server153) and package the result.
#
# Provenance and authorization: src/server153/PROVENANCE.md, docs/LEGAL.md.
# Toolchain: g++ -m32. On Debian 12 (CI image) this needs:
#   dpkg --add-architecture i386
#   apt-get install -y build-essential gcc-multilib g++-multilib \
#       libssl-dev:i386 libpcre3-dev:i386
#
# Usage:
#   scripts/build-server153.sh [--jobs N] [--src DIR] [--out DIR] [--only a,b]
#
# Stages: setup, perf, share-objs, share-io, logclient, gamed, gdbclient,
# cskill, libcommon, collision, cgame-lib, tasklib, gs, daemons (glinkd,
# gdeliveryd, gamedbd, gauthd, uniquenamed, gfaction, gacd, logservice),
# rpcgen (soft), package.
#
# Exit status: 0 only if every critical stage and every daemon binary builds.
# The report and the partial package are still written on failure.
set -uo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$REPO_ROOT/src/server153"
OUT="$REPO_ROOT/dist/server153"
JOBS=2
ONLY=""

while [ $# -gt 0 ]; do
  case "$1" in
    --jobs) JOBS="${2:?}"; shift 2 ;;
    --src)  SRC="${2:?}";  shift 2 ;;
    --out)  OUT="${2:?}";  shift 2 ;;
    --only) ONLY="${2:?}"; shift 2 ;;
    -h|--help) sed -n '2,20p' "$0"; exit 0 ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
done

mkdir -p "$OUT/logs"
REPORT="$OUT/BUILD-REPORT.txt"
: > "$REPORT"

log()   { printf '[%s] %s\n' "$(date -u +%H:%M:%S)" "$*" | tee -a "$REPORT"; }
pass()  { log "PASS  $*"; }
fail()  { log "FAIL  $*"; FAILCOUNT=$((FAILCOUNT + 1)); }
die()   { log "FATAL $*"; log "partial report and package kept in: $OUT"; exit 1; }

FAILCOUNT=0

want()  { [ -z "$ONLY" ] || [[ ",$ONLY," == *",$1,"* ]]; }

# run_stage <name> <critical|soft> <cmd...>
# Runs the command with output captured per stage; critical failure aborts.
# NOTE: rc is captured directly (an if/fi without else resets $? to 0).
run_stage() {
  local name="$1" kind="$2"; shift 2
  want "$name" || return 0
  log "STAGE $name"
  local start="$SECONDS"
  "$@" >"$OUT/logs/$name.log" 2>&1
  local rc=$?
  if [ "$rc" -eq 0 ]; then
    pass "$name ($((SECONDS - start))s)"
    return 0
  fi
  fail "$name (rc=$rc; log: logs/$name.log)"
  [ "$kind" = critical ] && die "stage $name failed"
  return 0
}

mk() { make -j"$JOBS" "$@"; }

log "pw-web server153 build — $(date -u +%Y-%m-%dT%H:%M:%SZ)"
log "src=$SRC out=$OUT jobs=$JOBS"
log "toolchain: $(g++ --version | head -1)"

# ---------------------------------------------------------------- preflight
[ -d "$SRC/cnet" ] && [ -d "$SRC/cgame" ] && [ -d "$SRC/cskill" ] \
  || die "source tree not found at $SRC"
grep -q 'RULES_DIR' "$SRC/cgame/Rules.make" \
  || die "cgame/Rules.make lacks the portable-path fix (see PROVENANCE.md)"
grep -q 'TOP_SRCDIR' "$SRC/cnet/glinkd/Makefile" \
  || die "cnet/glinkd/Makefile looks like a stub Makefile; wrong revision vendored?"

# ------------------------------------------------------------------- setup
setup_tree() {
  cd "$SRC"
  mkdir -p cgame/libgs/gs cgame/libgs/io cgame/libgs/db cgame/libgs/log
  (
    cd iolib
    rm -f libgsio.a libdbCli.a libgsPro2.a liblogCli.a libskill.a
    ln -s ../cnet/io/libgsio.a        libgsio.a
    ln -s ../cnet/gdbclient/libdbCli.a libdbCli.a
    ln -s ../cnet/gamed/libgsPro2.a    libgsPro2.a
    ln -s ../cnet/logclient/liblogCli.a liblogCli.a
    ln -s ../cskill/libskill.a         libskill.a
  )
  rm -f cskill/libskill.a cskill/libskill.so
  ln -s skill/libskill.a  cskill/libskill.a
  ln -s skill/libskill.so cskill/libskill.so
  ( cd cgame/gs && rm -f libskill.so && ln -s ../../cskill/libskill.so libskill.so )
}
run_stage setup critical setup_tree

# ------------------------------------------------- shared framework (share/)
run_stage perf       critical mk -C "$SRC/share/perf"
run_stage share-objs critical bash -c \
  "make -C '$SRC/share/io' pollio.o protocol.o security.o rpc.o proxyrpc.o && \
   make -C '$SRC/share/common' octets.o thread.o conf.o timer.o itimer.o"
run_stage share-io   critical mk -C "$SRC/share/io" lib

# ------------------------------------------------------- cnet static libs
run_stage logclient critical mk -C "$SRC/cnet/logclient" -f Makefile.gs lib
run_stage gamed     critical mk -C "$SRC/cnet/gamed" lib
run_stage gdbclient critical mk -C "$SRC/cnet/gdbclient" lib

# ---------------------------------------------------------------- cskill
build_cskill() {
  mk -C "$SRC/cskill/skill" -f Makefilelib lib
  mk -C "$SRC/cskill/skill" -f Makefilelib dlib
}
run_stage cskill critical build_cskill

# ------------------------------------------------------------------ cgame
run_stage libcommon critical mk -C "$SRC/cgame/libcommon"
run_stage collision critical mk -C "$SRC/cgame/collision"
run_stage cgame-lib critical mk -C "$SRC/cgame" lib
run_stage tasklib   critical mk -C "$SRC/cgame/gs/task" lib
run_stage gs        critical mk -C "$SRC/cgame/gs"

# ---------------------------------------------------------------- daemons
for d in glinkd gdeliveryd gamedbd gauthd uniquenamed gfaction gacd logservice; do
  run_stage "$d" soft mk -C "$SRC/cnet/$d"
done

# gclient: in-tree virtual client, used as the handshake probe against glinkd.
run_stage gclient soft mk -C "$SRC/cnet/gclient"

# ------------------------------------------------------------ rpcgen (soft)
# Validates that the committed cnet/rpcdata matches rpcalls.xml.
rpcgen_check() {
  cd "$SRC/cnet"
  command -v perl >/dev/null || { echo "perl not available; skipping"; return 0; }
  perl -MXML::DOM -e 1 2>/dev/null || { echo "perl XML::DOM not available; skipping"; return 0; }
  local tmp; tmp="$(mktemp -d)"
  cp -r rpcdata "$tmp/rpcdata.orig"
  ./rpcgen rpcalls.xml >/dev/null
  if diff -r "$tmp/rpcdata.orig" rpcdata >"$tmp/diff.txt" 2>&1; then
    echo "committed rpcdata matches rpcgen output"
  else
    echo "WARNING: committed rpcdata differs from rpcgen output:"
    head -40 "$tmp/diff.txt"
    return 1
  fi
  rm -rf "$tmp"
}
run_stage rpcgen soft rpcgen_check

# ---------------------------------------------------------------- package
package() {
  local pkg="$OUT/pw-server153"
  rm -rf "$pkg"
  mkdir -p "$pkg/bin" "$pkg/lib" "$pkg/conf"
  local f
  for f in cgame/gs/gs \
           cnet/glinkd/glinkd cnet/gdeliveryd/gdeliveryd cnet/gamedbd/gamedbd \
           cnet/gauthd/gauthd cnet/uniquenamed/uniquenamed cnet/gfaction/gfactiond \
           cnet/gacd/gacd cnet/logservice/logservice cnet/gclient/gclient; do
    [ -f "$SRC/$f" ] && cp "$SRC/$f" "$pkg/bin/" \
      || echo "MISSING binary: $f" >> "$REPORT"
  done
  for f in cskill/skill/libskill.a cskill/skill/libskill.so cgame/gs/libtask.so \
           cnet/io/libgsio.a cnet/gamed/libgsPro2.a cnet/gdbclient/libdbCli.a \
           cnet/logclient/liblogCli.a share/perf/libperf.a \
           cgame/libcommon.a cgame/libonline.a cgame/collision/libTrace.a; do
    [ -f "$SRC/$f" ] && cp "$SRC/$f" "$pkg/lib/" \
      || echo "MISSING lib: $f" >> "$REPORT"
  done
  ( cd "$SRC" && find cnet cgame -name '*.conf' -o -name 'gamesys.conf' -o -name '*.xml' \
      | while read -r f; do mkdir -p "$pkg/conf/$(dirname "$f")"; cp "$f" "$pkg/conf/$f"; done )
  cp "$REPORT" "$pkg/"
  {
    echo; echo "== product sizes =="
    du -sh "$pkg/bin" "$pkg/lib" "$pkg/conf" 2>/dev/null
    ls -l "$pkg/bin" "$pkg/lib"
  } >> "$REPORT"
  tar -C "$OUT" -czf "$OUT/pw-server153-$(git -C "$REPO_ROOT" rev-parse --short HEAD).tar.gz" \
    "$(basename "$pkg")"
  log "package: $OUT/pw-server153-*.tar.gz"
}
run_stage package critical package

if [ "$FAILCOUNT" -gt 0 ]; then
  log "BUILD INCOMPLETE — $FAILCOUNT failed stage(s); see $REPORT"
  exit 1
fi
log "BUILD OK — package and report in $OUT"
