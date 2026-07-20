#!/usr/bin/env bash
# Real-window E2E test for SuperTux (SUPERSET of the xvfb smoke suite).
#
# The existing tests/e2e/*_smoke.sh run UNDER xvfb (Mesa software GL). On a
# real XWayland/GPU display SuperTux SDL3 has historically booted and exited
# cleanly (all xvfb tests green) while rendering a COMPLETELY BLACK SCREEN --
# a regression no log/exit-code check catches. Only a real pixel check on the
# actual :0 framebuffer does. Reproduced on NVIDIA + SDL3: main menu AND
# level both render mean=0 (black) while the log reports "In main menu" /
# "Watching a cutscene" with no crash.
#
# This test runs the built binary on the REAL DISPLAY (no xvfb-run), loads a
# real level + main menu, lets frames render, screenshots the actual :0
# framebuffer with ffmpeg x11grab, and asserts the framebuffer is NOT
# uniformly black (ImageMagick mean pixel value). It also hard-fails on any
# crash/error in the log (signal 11 / TTF_Close / FT_Done -- AGENTS.md rule).
#
# Intended for LOCAL verification on a machine with a live X session (:0).
# CI stays on the xvfb-based smoke tests (headless, no real display).
#
# Usage: e2e_real_window.sh <path-to-supertux2> [level.stl]
set -u

SUPERTUX="${1:-}"
if [ -z "$SUPERTUX" ] || [ ! -x "$SUPERTUX" ]; then
  echo "ERROR: supertux2 binary not found or not executable: '$SUPERTUX'" >&2
  exit 2
fi
shift || true

SRC_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
LEVEL="${1:-$SRC_DIR/data/levels/bonus1/refisherator.stl}"

# Real-display tooling.
for tool in ffmpeg convert; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "SKIP: '$tool' not found; cannot run real-window check" >&2
    exit 0
  fi
done
if [ -z "${DISPLAY:-}" ]; then
  echo "SKIP: no DISPLAY set; real-window test needs a live X session" >&2
  exit 0
fi
if echo "$DISPLAY" | grep -q ':99'; then
  echo "SKIP: running under xvfb (DISPLAY=$DISPLAY); use the xvfb smoke tests instead" >&2
  exit 0
fi

export SDL_VIDEODRIVER="${SDL_VIDEODRIVER:-x11}"

LOG="$(mktemp)"
SHOT="$(mktemp --suffix=.png)"
trap 'rm -f "$LOG" "$SHOT"' EXIT

capture() {
  # ffmpeg x11grab reliably captures the real :0 framebuffer (scrot fails
  # under SDL3 fullscreen / some compositors). Sample the root window.
  ffmpeg -y -f x11grab -s 1280x720 -i "${DISPLAY}.0" -frames:v 1 "$SHOT" >/dev/null 2>&1
}

mean_of() {
  [ -s "$SHOT" ] || { echo 0; return; }
  convert "$SHOT" -format "%[mean]" info: 2>/dev/null | awk '{print $1}'
}

echo "=== real-window E2E: $SUPERTUX (DISPLAY=$DISPLAY) ==="

# 1) Main menu render check.
"$SUPERTUX" --verbose >"$LOG" 2>&1 &
PID=$!
sleep 6
capture
MENU_MEAN=$(mean_of)
kill "$PID" 2>/dev/null; wait "$PID" 2>/dev/null
echo "main-menu mean pixel: $MENU_MEAN"
if grep -iqE "signal 11|TTF_Close|FT_Done|Error:" "$LOG" 2>/dev/null; then
  echo "FAIL: supertux2 logged a crash/error during main-menu run:" >&2
  grep -iE "signal 11|TTF_Close|FT_Done|Error:" "$LOG" >&2
  exit 1
fi

# 2) Level render check.
if [ -f "$LEVEL" ]; then
  : >"$LOG"
  "$SUPERTUX" "$LEVEL" --fullscreen --verbose >"$LOG" 2>&1 &
  PID=$!
  LOADED=0
  for _ in $(seq 1 60); do
    if grep -qE "Watching a cutscene|In level|Loading level|sector" "$LOG" 2>/dev/null; then
      LOADED=1; break
    fi
    kill -0 "$PID" 2>/dev/null || break
    sleep 0.25
  done
  if [ "$LOADED" -eq 0 ]; then
    echo "FAIL: level did not load within timeout" >&2
    tail -n 15 "$LOG" >&2
    kill "$PID" 2>/dev/null; wait "$PID" 2>/dev/null
    exit 1
  fi
  sleep 3
  capture
  LEVEL_MEAN=$(mean_of)
  kill "$PID" 2>/dev/null; wait "$PID" 2>/dev/null
  echo "level mean pixel: $LEVEL_MEAN"
  if grep -iqE "signal 11|TTF_Close|FT_Done|Error:" "$LOG" 2>/dev/null; then
    echo "FAIL: supertux2 logged a crash/error during level run:" >&2
    grep -iE "signal 11|TTF_Close|FT_Done|Error:" "$LOG" >&2
    exit 1
  fi
else
  echo "SKIP: level not found: $LEVEL"
  LEVEL_MEAN=0
fi

# Pixel guard: assert BOTH the menu and (if loaded) the level rendered
# non-black frames. A uniformly black framebuffer on a real GPU display is
# the SDL3 black-screen regression this test exists to catch.
RC=0
if awk "BEGIN { exit !($MENU_MEAN > 0.5) }"; then
  echo "PASS: main menu rendered non-black frame (mean=$MENU_MEAN)"
else
  echo "FAIL: main menu rendered a (near-)black screen (mean=$MENU_MEAN)" >&2
  RC=1
fi
if [ -f "$LEVEL" ]; then
  if awk "BEGIN { exit !($LEVEL_MEAN > 0.5) }"; then
    echo "PASS: level rendered non-black frame (mean=$LEVEL_MEAN)"
  else
    echo "FAIL: level rendered a (near-)black screen (mean=$LEVEL_MEAN)" >&2
    RC=1
  fi
fi
exit $RC
