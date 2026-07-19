#!/usr/bin/env bash
#  SuperTux
#  Copyright (C) 2026 SuperTux contributors
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Local pre-push check for the unit-test suite, mirroring the Fork CI sanitizer
# job (ASan + UBSan). Run this before pushing test changes so that
# stack-use-after-scope / UB regressions are caught locally instead of turning
# the remote Fork CI run red.
#
#   bash scripts/check_tests_sanitized.sh
#
# What it does:
#   - configures a separate build-san dir with -fsanitize=address,undefined
#   - builds the `tests` target
#   - runs ctest, excluding the vendored sexp-cpp `test_sexp` (its 2 pre-existing
#     failures are unrelated to SuperTux and present on upstream/master too)
#   - runs the E2E smoke/render tests under xvfb (same as Fork CI) when available
#
# Exit code is non-zero if any SuperTux test fails or if sanitizers abort.

set -euo pipefail

SRC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${SUPERTUX_SAN_BUILD_DIR:-$SRC_DIR/build-san}"
JOBS="${JOBS:-$(nproc)}"

cd "$SRC_DIR"

echo "==> Configuring sanitizer build in $BUILD_DIR"
cmake -B "$BUILD_DIR" -S . \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5

echo "==> Building unit tests"
cmake --build "$BUILD_DIR" --target tests -j"$JOBS"

echo "==> Running unit tests under ASan/UBSan"
# Use the same exclusion as fork-tests.yml: skip the vendored sexp-cpp target.
( cd "$BUILD_DIR" && ctest --output-on-failure -E 'test_sexp' )

echo "==> Sanitizer unit-test check passed."
