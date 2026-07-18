#!/usr/bin/env bash
# Convenience launcher for SuperTux (SDL3 fork).
#
# SuperTux SDL3 renders a black framebuffer on some X11/XWayland setups
# (notably XWayland under mutter) while the game logic boots fine. On
# Wayland sessions it starts cleanly; using the native Wayland video
# driver avoids the XWayland present problem. This wrapper picks the best
# available video driver so the game is actually visible:
#
#   - if WAYLAND_DISPLAY is set, use SDL_VIDEODRIVER=wayland
#   - otherwise fall back to x11 (and let xvfb-run be used by CI)
#
# Usage: ./run.sh [extra supertux2 args]

set -u

# Resolve the directory this script lives in, so it works from anywhere.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SUPERTUX_BIN="${SCRIPT_DIR}/build-sdl3/supertux2"

if [ ! -x "${SUPERTUX_BIN}" ]; then
  echo "ERROR: supertux2 binary not found or not executable: ${SUPERTUX_BIN}" >&2
  echo "Build it first, e.g.: cmake -B build-sdl3 -S . -DBUILD_TESTING=ON && cmake --build build-sdl3 --target supertux2" >&2
  exit 2
fi

# Prefer Wayland when a Wayland session is available; it renders visibly
# where XWayland presents a black framebuffer.
if [ -n "${SDL_VIDEODRIVER:-}" ]; then
  : # user override wins
elif [ -n "${WAYLAND_DISPLAY:-}" ]; then
  export SDL_VIDEODRIVER=wayland
else
  export SDL_VIDEODRIVER=x11
fi

# On some systems the default OpenGL renderer presents a black framebuffer
# (SDL3 + certain Mesa/NVIDIA drivers under Wayland/XWayland) while the
# software renderer works. Default to software unless overridden.
if [ -z "${SUPERTUX_RENDERER:-}" ]; then
  export SUPERTUX_RENDERER=software
fi

echo "Launching SuperTux with SDL_VIDEODRIVER=${SDL_VIDEODRIVER} (DISPLAY=${DISPLAY:-}, WAYLAND_DISPLAY=${WAYLAND_DISPLAY:-})"
exec "${SUPERTUX_BIN}" "$@"
