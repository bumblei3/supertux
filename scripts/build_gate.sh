#!/usr/bin/env bash
# Local mirror of the fork-tests.yml CI matrix: build the unit tests and run
# them under (a) plain Release, (b) ASan+UBSan, (c) Coverage. This lets you
# catch regressions before pushing, exactly as CI would.
#
# Usage:  ./scripts/build_gate.sh [--no-coverage]
set -u

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT" || exit 1

SANITIZERS="address,undefined"
COVERAGE=1
[ "${1:-}" = "--no-coverage" ] && COVERAGE=0

run_gate() {
  local label="$1"; shift
  local cxx_flags="$1"; shift
  echo "================================================================"
  echo ">> GATE: $label"
  echo ">> CXX_FLAGS: ${cxx_flags:-<none>}"
  echo "================================================================"
  rm -rf build-gate
  mkdir -p build-gate
  ( cd build-gate || exit 1
    cmake -G "Unix Makefiles" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DBUILD_TESTING=ON \
      -DBUILD_TESTS=ON \
      ${cxx_flags:+-DCMAKE_CXX_FLAGS="$cxx_flags"} \
      .. || { echo "CMAKE FAILED: $label"; exit 1; }
    cmake --build . --target tests -j"$(nproc)" || { echo "BUILD FAILED: $label"; exit 1; }
    # external/sexp-cpp is a vendored submodule with 2 pre-existing,
    # unrelated test_sexp failures present on upstream/master too. Run only
    # SuperTux's own unit tests.
    ctest --output-on-failure -E 'test_sexp' || { echo "CTEST FAILED: $label"; exit 1; }
  )
  echo ">> GATE PASSED: $label"
}

# (a) Plain Debug build + tests.
run_gate "release (plain)" ""

# (b) AddressSanitizer + UndefinedBehaviorSanitizer.
run_gate "asan+ubsan" "-fsanitize=${SANITIZERS} -fno-omit-frame-pointer"

# (c) Coverage (gcov). Generates build-gate/coverage.info via gcovr if present.
if [ "$COVERAGE" -eq 1 ]; then
  run_gate "coverage (gcov)" "--coverage -fno-omit-frame-pointer"
  if command -v gcovr >/dev/null 2>&1; then
    ( cd build-gate || exit 1
      gcovr --object-directory . --root "$REPO_ROOT" \
        --filter "$REPO_ROOT/src/.*" \
        --print-summary || true
    )
  else
    echo ">> (gcovr not installed locally; coverage .gcda files are in build-gate/)"
  fi
fi

echo "ALL GATES PASSED"
