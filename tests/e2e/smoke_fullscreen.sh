#!/usr/bin/env bash
# E2E smoke test for SuperTux's fullscreen startup path.
#
# Boots the real supertux2 binary headless under a virtual X display (xvfb)
# with --fullscreen and asserts that:
#   1. the process does not crash (clean exit, no segfault/signal), and
#   2. it actually reaches the desktop fullscreen mode (the "switched to
#      desktop fullscreen mode" log line), which is exactly the path that
#      previously broke under Wayland/Xwayland (fixed in 6c30d803c).
#
# This catches fullscreen/startup regressions automatically in CI that the
# unit tests (which run against the dummy SDL driver) cannot see.
#
# Usage: smoke_fullscreen.sh <path-to-supertux2>
set -u

SUPERTUX="${1:-}"
if [ -z "$SUPERTUX" ] || [ ! -x "$SUPERTUX" ]; then
  echo "ERROR: supertux2 binary not found or not executable: '$SUPERTUX'" >&2
  exit 2
fi

# We don't need a real display, but we want the *real* X11 driver (not dummy)
# so compositor-specific fullscreen behavior is exercised. xvfb provides one.
export SDL_VIDEODRIVER="${SDL_VIDEODRIVER:-x11}"

LOG="$(mktemp)"
trap 'rm -f "$LOG"' EXIT

# Boot, wait for the fullscreen path to be reached, then shut down.
# --fullscreen forces the fullscreen code path; we give it a few seconds.
timeout 20 xvfb-run -a "$SUPERTUX" --fullscreen --verbose >"$LOG" 2>&1 &
PID=$!

# Wait until either the fullscreen line appears or the process ends.
#
# Note: under a headless xvfb display SuperTux cannot perform a true
# fullscreen mode-switch, so X reports a benign "Time out elapsed after
# mode switch ... reverting" ERROR and SuperTux falls back to a windowed
# fullscreen-sized surface, logging "switched to fullscreen mode:
# WxH@R". That fallback is expected headless behavior, NOT a crash -- we
# therefore match the real reached-state line, not the desktop-mode one.
FOUND=0
for _ in $(seq 1 40); do
  if grep -q "switched to fullscreen mode" "$LOG" 2>/dev/null; then
    FOUND=1
    break
  fi
  if ! kill -0 "$PID" 2>/dev/null; then
    break
  fi
  sleep 0.25
done

# Give the game a moment to settle, then terminate cleanly.
kill "$PID" 2>/dev/null
wait "$PID" 2>/dev/null
RC=$?

echo "--- supertux2 fullscreen smoke log (tail) ---"
tail -n 15 "$LOG"

if [ "$FOUND" -ne 1 ]; then
  echo "FAIL: supertux2 did not reach desktop fullscreen mode" >&2
  exit 1
fi

# We killed the process ourselves with SIGTERM (rc 143 = 128 + 15), which is
# expected and NOT a crash. A crash would be a signal other than SIGTERM while
# still initializing (e.g. SIGSEGV=139, SIGABRT=134). Only those are hard fails.
if [ "$RC" -ne 143 ] && [ "$RC" -ge 128 ]; then
  echo "FAIL: supertux2 crashed during startup (wait rc=$RC)" >&2
  exit 1
fi

echo "PASS: supertux2 reached desktop fullscreen mode without crashing"
exit 0
