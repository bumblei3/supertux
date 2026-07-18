#!/usr/bin/env bash
# E2E render-pixel smoke test for SuperTux.
#
# Extends the boot smoke test (smoke_fullscreen.sh) with an actual
# RENDER-OUTPUT check. The boot smoke test only verifies the process
# reaches fullscreen mode and exits cleanly -- it cannot catch a
# "black screen" regression where the game runs but nothing is drawn
# (e.g. an SDL3 renderer/display/compositor bug that presents an empty
# framebuffer). This test captures the X framebuffer via scrot and
# asserts it is NOT uniformly black, using ImageMagick's mean pixel
# value.
#
# Why this matters: on a real XWayland/GPU display SuperTux SDL3 booted
# and exited cleanly (all existing tests green) while rendering a
# completely black screen. No log/exit-code check caught it; only a
# pixel check does. Under xvfb (Mesa software) SuperTux DOES render
# visible pixels, so this test is meaningful in CI.
#
# Usage: smoke_render_pixels.sh <path-to-supertux2>

set -u

SUPERTUX="${1:-}"
if [ -z "$SUPERTUX" ] || [ ! -x "$SUPERTUX" ]; then
  echo "ERROR: supertux2 binary not found or not executable: '$SUPERTUX'" >&2
  exit 2
fi

# Tools we depend on.
for tool in xvfb-run scrot convert; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "SKIP: '$tool' not found; cannot run render-pixel check" >&2
    exit 0
  fi
done

export SDL_VIDEODRIVER="${SDL_VIDEODRIVER:-x11}"

LOG="$(mktemp)"
SHOT="$(mktemp --suffix=.png)"
trap 'rm -f "$LOG" "$SHOT"' EXIT

# Run inside xvfb-run so DISPLAY + XAUTHORITY are correctly set for both
# the game and the scrot screenshot. We background the game, wait for the
# menu, capture, then terminate cleanly.
xvfb-run -a bash -c "
  SDL_VIDEODRIVER=x11 '$SUPERTUX' --fullscreen --verbose >'$LOG' 2>&1 &
  PID=\$!
  for _ in \$(seq 1 40); do
    grep -q 'In worldmap\|In main menu' '$LOG' 2>/dev/null && break
    kill -0 \"\$PID\" 2>/dev/null || break
    sleep 0.25
  done
  sleep 1
  rm -f '$SHOT'
  scrot '$SHOT' 2>/dev/null
  kill '\$PID' 2>/dev/null
  wait '\$PID' 2>/dev/null
" 2>&1

echo "--- supertux2 render smoke log (tail) ---"
tail -n 15 "$LOG"

# Regression guard: a crash/error in the log is a hard fail even if we
# somehow captured a non-black frame. (Catches e.g. the TTF_CloseFont /
# FT_Done_Face font-close crash, or any signal 11 during shutdown.)
if grep -iqE "signal 11|Error:" "$LOG" 2>/dev/null; then
  echo "FAIL: supertux2 logged a crash/error during run:" >&2
  grep -iE "signal 11|Error:" "$LOG" >&2
  exit 1
fi

# The actual render check: the captured framebuffer must not be black.
if [ ! -s "$SHOT" ]; then
  echo "FAIL: no screenshot captured (rendering may have failed to present)" >&2
  exit 1
fi

MEAN="$(convert "$SHOT" -format "%[mean]" info: 2>/dev/null || echo 0)"
# ImageMagick reports mean on a 0-65535 scale. A black frame is 0; a
# rendered SuperTux frame is well above ~5000. Use a conservative floor.
if [ -z "$MEAN" ] || [ "${MEAN%.*}" -lt 1000 ]; then
  echo "FAIL: captured framebuffer is (near) black (mean=$MEAN); rendering produced no visible output" >&2
  exit 1
fi

echo "PASS: supertux2 rendered visible output (mean=$MEAN) without crashing"
exit 0
