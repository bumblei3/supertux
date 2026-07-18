#!/usr/bin/env bash
# E2E test: verify the SDL_Renderer driver can be selected via
# SUPERTUX_RENDERER (regression guard for the black-screen fix).
#
# On some systems the default OpenGL SDL3 renderer presents a black
# framebuffer (SDL3 + certain Mesa/NVIDIA drivers under Wayland/XWayland)
# while the software renderer works. sdl_video_system.cpp honours
# SUPERTUX_RENDERER to pick the driver. This test asserts that the
# override actually takes effect (the log line "SDL_Renderer: <name>"
# in sdl_screen_renderer.cpp), so a future regression that ignores the
# env var (or hardcodes opengl) is caught.
#
# Usage: smoke_renderer_choice.sh <path-to-supertux2>

set -u

SUPERTUX="${1:-}"
if [ -z "$SUPERTUX" ] || [ ! -x "$SUPERTUX" ]; then
  echo "ERROR: supertux2 binary not found or not executable: '$SUPERTUX'" >&2
  exit 2
fi

for tool in xvfb-run; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "SKIP: '$tool' not found" >&2
    exit 0
  fi
done

export SDL_VIDEODRIVER="${SDL_VIDEODRIVER:-x11}"

# $1 = renderer name to request via SUPERTUX_RENDERER
# $2 = expected substring in the "SDL_Renderer:" log line
check_renderer() {
  local want="$1" expect="$2"
  local LOG; LOG="$(mktemp)"
  SUPERTUX_RENDERER="$want" xvfb-run -a bash -c "
    '$SUPERTUX' --fullscreen --verbose >'$LOG' 2>&1 &
    PID=\$!
    for _ in \$(seq 1 40); do
      grep -q 'In main menu\|In worldmap' '$LOG' 2>/dev/null && break
      kill -0 \"\$PID\" 2>/dev/null || break
      sleep 0.25
    done
    kill '\$PID' 2>/dev/null; wait '\$PID' 2>/dev/null
  " 2>&1
  local got
  got="$(grep -oE 'SDL_Renderer: [a-z]+' "$LOG" | head -1 | awk '{print $2}')"
  rm -f "$LOG"
  if [ "$got" = "$expect" ]; then
    echo "PASS: SUPERTUX_RENDERER=$want -> renderer '$got'"
    return 0
  else
    echo "FAIL: SUPERTUX_RENDERER=$want -> expected '$expect' but got '$got'" >&2
    return 1
  fi
}

RC=0
check_renderer "software" "software" || RC=1
check_renderer "opengl"   "opengl"   || RC=1
exit $RC
