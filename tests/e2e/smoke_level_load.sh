#!/usr/bin/env bash
# E2E level-load smoke test for SuperTux.
#
# The boot/worldmap E2E tests only exercise the menu and (via CLI) the
# worldmap path. This test exercises the LEVEL LOADING code path
# (LevelParser, GameSession init, sector setup) by launching the real
# binary with one or more .stl level files as arguments. It does NOT
# assert on rendered pixels: under a headless xvfb display SuperTux
# plays an intro cutscene that does not always advance, so the framebuffer
# may be black -- that is fine. What matters here is that loading a level
# and setting up its sectors/game session must NOT crash the process.
#
# This catches regressions in level parsing / GameSession construction
# that the menu/worldmap E2E paths never reach. (Note: passing a .stwm
# worldmap directly DOES crash in WorldMapState::new_save -- that path is
# intentionally not tested here; worldmaps are reached via the in-game
# menu, not the CLI.)
#
# Usage: smoke_level_load.sh <path-to-supertux2> <source-dir>

set -u

SUPERTUX="${1:-}"
if [ -z "$SUPERTUX" ] || [ ! -x "$SUPERTUX" ]; then
  echo "ERROR: supertux2 binary not found or not executable: '$SUPERTUX'" >&2
  exit 2
fi
shift

# Source dir (repo root) so level paths under data/levels/ resolve wherever
# ctest is invoked from (CI runs ctest inside the build dir, not the root).
SRC_DIR="${1:-$(cd "$(dirname "$0")/../.." && pwd)}"
if [ ! -d "$SRC_DIR/data/levels" ]; then
  echo "SKIP: data/levels not found under '$SRC_DIR'" >&2
  exit 0
fi
shift || true

# Default level set if none provided (resolved against SRC_DIR).
if [ "$#" -eq 0 ]; then
  set -- \
    "$SRC_DIR/data/levels/bonus1/refisherator.stl" \
    "$SRC_DIR/data/levels/revenge_in_redmond/where_my_super_cape.stl"
fi

for tool in xvfb-run; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "SKIP: '$tool' not found; cannot run level-load check" >&2
    exit 0
  fi
done

export SDL_VIDEODRIVER="${SDL_VIDEODRIVER:-x11}"

RC=0
for lvl in "$@"; do
  if [ ! -f "$lvl" ]; then
    echo "SKIP: level not found: $lvl"
    continue
  fi
  LOG="$(mktemp)"
  trap 'rm -f "$LOG"' EXIT

  xvfb-run -a bash -c "
    '$SUPERTUX' --fullscreen --verbose '$lvl' >'$LOG' 2>&1 &
    PID=\$!
    # Give the level time to load + (possibly) start a cutscene. We do not
    # wait for 'In game' -- the cutscene may not advance headless. We only
    # need the load/setup path to not crash.
    sleep 5
    kill '\$PID' 2>/dev/null
    wait '\$PID' 2>/dev/null
  " 2>&1

  if grep -iqE "signal 11|XIOError|XCloseDisplay|_XDefaultIOError|Error:" "$LOG" 2>/dev/null; then
    echo "FAIL: loading '$lvl' logged a crash/error:" >&2
    grep -iE "signal 11|XIOError|XCloseDisplay|_XDefaultIOError|Error:" "$LOG" >&2
    RC=1
  else
    echo "PASS: level loaded without crash: $lvl"
  fi
  rm -f "$LOG"
done

exit $RC
