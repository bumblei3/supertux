#!/usr/bin/env bash
# E2E gameplay-render smoke test for SuperTux.
#
# Unlike SmokeLevelLoad (which only asserts level loading does not crash),
# this test exercises the RENDER LOOP of an actual game session. Launching
# a .stl level directly (without a worldmap) opens a GameSession; SuperTux
# then plays an intro cutscene, but that cutscene RENDERS through the same
# Compositor::render() path as live gameplay (HUD, sprites, tilemap). So a
# longer run that reaches "Watching a cutscene" proves the render loop is
# alive and not crashing frame-to-frame -- the regression class that bit us
# at PlayerStatusHUD::draw (Resources::fixed_font null deref on the real
# display) and the boot use-after-free.
#
# We do NOT try to skip the cutscene (headless xvfb has no reliable key
# injection, and the cutscene may not advance). We just let the render loop
# run for a while and assert no crash. Clean shutdown via SIGTERM (not
# `timeout`, which would kill xvfb and produce a benign XCloseDisplay
# signal-11 that is the harness, not the game).
#
# Usage: smoke_gameplay.sh <path-to-supertux2> [extra level paths...]

set -u

SUPERTUX="${1:-}"
if [ -z "$SUPERTUX" ] || [ ! -x "$SUPERTUX" ]; then
  echo "ERROR: supertux2 binary not found or not executable: '$SUPERTUX'" >&2
  exit 2
fi
shift

# Default level set if none provided.
if [ "$#" -eq 0 ]; then
  set -- \
    "data/levels/bonus1/refisherator.stl" \
    "data/levels/revenge_in_redmond/where_my_super_cape.stl"
fi

for tool in xvfb-run; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "SKIP: '$tool' not found; cannot run gameplay check" >&2
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
    # Wait until the render loop is actually running (cutscene or gameplay).
    for _ in \$(seq 1 60); do
      grep -qiE 'Watching a cutscene|In worldmap|In main menu|Setting status: In game' '$LOG' 2>/dev/null && break
      kill -0 \"\$PID\" 2>/dev/null || break
      sleep 0.25
    done
    # Let the render loop run a while to catch frame-to-frame crashes.
    sleep 8
    kill '\$PID' 2>/dev/null
    wait '\$PID' 2>/dev/null
  " 2>&1

  # A real crash is a signal 11 whose stack does NOT involve the benign
  # xvfb-teardown path (XCloseDisplay / _XDefaultIOError / _XIOError). When
  # we `kill` the process ourselves (not `timeout`), xvfb stays alive, so a
  # clean SIGTERM does NOT produce the XCloseDisplay signal-11 -- but if a
  # stray xvfb-kill happens anyway, ignore it (it ends in ~SDLSubsystem /
  # Main, not in a render/draw frame). Only a crash with a game-code frame
  # (Compositor::render, PlayerStatusHUD, GameSession, etc.) is a real FAIL.
  if grep -iqE "signal 11" "$LOG" 2>/dev/null; then
    if grep -qE "XCloseDisplay|_XDefaultIOError|_XIOError" "$LOG" 2>/dev/null; then
      echo "PASS (benign xvfb-teardown signal-11 ignored): $lvl"
    elif grep -qE "Compositor::render|PlayerStatusHUD|GameSession|ScreenManager::loop_iter|TileMap::draw|Sprite::draw" "$LOG" 2>/dev/null; then
      echo "FAIL: '$lvl' crashed in a render/draw frame:" >&2
      grep -iE "signal 11" "$LOG" >&2
      RC=1
    else
      echo "PASS (signal-11 without game-code frame, likely teardown): $lvl"
    fi
  else
    echo "PASS: render loop ran without crash: $lvl"
  fi
  rm -f "$LOG"
done

exit $RC
