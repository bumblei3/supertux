#!/usr/bin/env bash
# Real-window E2E guard — catches the SDL3 black-screen regression on a real
# GPU/X display that the xvfb smoke suite (Mesa software GL) cannot see.
#
# The harness boots the actual binary on :0 (no xvfb), captures the
# framebuffer with ffmpeg x11grab, and asserts the mean pixel value is > 0.5
# (non-black). A black framebuffer is invisible to every log/exit-code check,
# so only a pixel sample on the real display isolates it.
#
# LOCAL ONLY. CI is headless and must not run this (see the skip guard below).
set -uo pipefail

SUPERTUX="${SUPERTUX:-./build/supertux2}"

# --- Skip guard: never run under xvfb / headless CI -------------------------
if [[ "${DISPLAY:-}" == ":99" ]] || [[ -n "${XVFB_RUN_PID:-}" ]]; then
  echo "SKIP: running under xvfb/headless CI; this harness needs a real :0"
  exit 0
fi
# On Wayland + rootless Xwayland, ffmpeg x11grab does NOT reliably map the
# X11 root onto the physical monitors (proven: a region with visible content
# reports mean=0). A pixel guard would be a permanent false-positive there,
# so we skip instead of failing. The guard still runs on genuine X11 sessions
# (e.g. the NVIDIA X11 box where the SDL3 black-screen was first observed).
if [[ "${XDG_SESSION_TYPE:-}" == "wayland" ]]; then
  echo "SKIP: Wayland session (rootless Xwayland) — ffmpeg x11grab is unreliable here;"
  echo "      run this harness on a real X11 :0 to validate the SDL3 black-screen guard."
  exit 0
fi
if ! command -v ffmpeg >/dev/null 2>&1; then
  echo "SKIP: ffmpeg not installed"
  exit 0
fi
if ! command -v convert >/dev/null 2>&1; then
  echo "SKIP: ImageMagick 'convert' not installed"
  exit 0
fi
if [[ ! -x "$SUPERTUX" ]]; then
  echo "ERROR: supertux2 binary not found at '$SUPERTUX'" >&2
  exit 1
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
LOGFILE="$WORK/supertux.log"
SHOT="$WORK/shot.png"

# Level is POSITIONAL: supertux2 [OPTIONEN] [LEVEL], NOT --level.
LEVEL="${LEVEL:-}"

fail() { echo "FAIL: $*" >&2; exit 1; }

mean_of() {
  convert "$1" -format "%[mean]" info: 2>/dev/null || echo 0
}

# --- Boot the real binary ---------------------------------------------------
echo "Launching $SUPERTUX on DISPLAY=${DISPLAY:-:0} ..."
"$SUPERTUX" $LEVEL --fullscreen --verbose >"$LOGFILE" 2>&1 &
PID=$!
cleanup() { kill "$PID" 2>/dev/null || true; }
trap 'cleanup; rm -rf "$WORK"' EXIT

# --- Wait for the main menu / level to load --------------------------------
DEADLINE=$(( $(date +%s) + 30 ))
LOADED=0
while [[ $(date +%s) -lt $DEADLINE ]]; do
  if grep -qE "In main menu|Watching a cutscene|In level|Loading level|sector" "$LOGFILE" 2>/dev/null; then
    LOADED=1
    break
  fi
  if ! kill -0 "$PID" 2>/dev/null; then
    echo "Process exited early. Log:"; cat "$LOGFILE"
    fail "supertux2 exited before rendering"
  fi
  sleep 0.5
done
[[ $LOADED -eq 1 ]] || fail "timed out waiting for main menu / level load"

# --- Sample the framebuffer at t≈+4s (main menu) ---------------------------
sleep 4
ffmpeg -y -f x11grab -s 1280x720 -i "${DISPLAY:-:0}.0" -frames:v 1 "$SHOT" >/dev/null 2>&1 || fail "ffmpeg capture failed"
[[ -s "$SHOT" ]] || fail "captured framebuffer is empty (size 0)"
MENU_MEAN="$(mean_of "$SHOT")"
MENU_OK=$(awk "BEGIN{exit !($MENU_MEAN > 0.5)}" && echo yes || echo no)
echo "main-menu mean pixel: $MENU_MEAN (non-black: $MENU_OK)"

# --- If a level was given, sample again after load -------------------------
if [[ -n "$LEVEL" ]]; then
  # wait for the level itself to settle
  sleep 4
  ffmpeg -y -f x11grab -s 1280x720 -i "${DISPLAY:-:0}.0" -frames:v 1 "$SHOT" >/dev/null 2>&1 || fail "ffmpeg capture failed"
  LEVEL_MEAN="$(mean_of "$SHOT")"
  LEVEL_OK=$(awk "BEGIN{exit !($LEVEL_MEAN > 0.5)}" && echo yes || echo no)
  echo "level mean pixel: $LEVEL_MEAN (non-black: $LEVEL_OK)"
fi

# --- Crash guard ------------------------------------------------------------
if grep -qE "signal 11|TTF_Close|FT_Done|Error:" "$LOGFILE" 2>/dev/null; then
  fail "crash/error signature in log"
fi

# --- Verdict ----------------------------------------------------------------
if [[ "$MENU_OK" != "yes" ]]; then
  fail "main-menu renders black (mean=$MENU_MEAN) — SDL3 black-screen regression"
fi
if [[ -n "$LEVEL" && "$LEVEL_OK" != "yes" ]]; then
  fail "level renders black (mean=$LEVEL_MEAN) — SDL3 black-screen regression"
fi

echo "PASS: real-window render is non-black on ${DISPLAY:-:0}"
cleanup
exit 0
