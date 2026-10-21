#!/usr/bin/env bash
# E2E editor smoke test for SuperTux.
#
# Boots the real binary in EDITOR mode on a .stl level (--edit-level) under
# xvfb and asserts it does not crash. Exercises the editor code path
# (Editor, OverlayWidget, tile converters) which the gameplay E2E never
# reaches. Only asserts no crash; does not check rendered pixels.
#
# Usage: smoke_editor.sh <path-to-supertux2> <source-dir>

set -u

SUPERTUX="${1:-}"
if [ -z "$SUPERTUX" ] || [ ! -x "$SUPERTUX" ]; then
  echo "ERROR: supertux2 binary not found or not executable: '$SUPERTUX'" >&2
  exit 2
fi
shift

SRC_DIR="${1:-$(cd "$(dirname "$0")/../.." && pwd)}"
if [ ! -d "$SRC_DIR/data/levels" ]; then
  echo "SKIP: data/levels not found under '$SRC_DIR'" >&2
  exit 0
fi
shift || true

if [ "$#" -eq 0 ]; then
  set -- \
    "$SRC_DIR/data/levels/bonus1/refisherator.stl"
fi

for tool in xvfb-run; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "SKIP: '$tool' not found; cannot run editor check" >&2
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
    '$SUPERTUX' --fullscreen --verbose --edit-level '$lvl' >'$LOG' 2>&1 &
    PID=\$!
    sleep 6
    kill '\$PID' 2>/dev/null
    wait '\$PID' 2>/dev/null
  " 2>&1

  if grep -iqE "signal 11|XIOError|XCloseDisplay|_XDefaultIOError" "$LOG" 2>/dev/null; then
    echo "FAIL: editor on '$lvl' crashed:" >&2
    grep -iE "signal 11|XIOError|XCloseDisplay|_XDefaultIOError" "$LOG" >&2
    RC=1
  else
    echo "PASS: editor ran without crash: $lvl"
  fi
  rm -f "$LOG"
done

exit $RC
EOF
chmod +x /home/tobber/supertux/tests/e2e/smoke_editor.sh
echo created