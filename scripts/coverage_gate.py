#!/usr/bin/env python3
"""Per-file coverage gate for the SuperTux (bumblei3) fork.

Reads a gcovr XML report (produced with `--xml coverage.xml`) and enforces
minimum line-coverage thresholds per source file. This turns the fork's
measured unit-test coverage into a hard CI gate so a later change that
silently drops coverage (e.g. deleting a test) fails CI.

Note on thresholds: the fork's unit tests are split across multiple binaries
that share some sources (e.g. StringUtilTest and StringUtilStTest both
compile src/util/string_util.cpp). gcovr therefore under-reports per-file
coverage in the aggregated run. Thresholds are set BELOW the measured
baseline so the gate catches real regressions without false positives from
that artifact.

Usage:
    coverage_gate.py coverage.xml [--threshold FILE:MIN ...]
"""

import sys
import argparse
import xml.etree.ElementTree as ET


# Measured baseline (build-cov, aggregated gcovr run):
#   src/collision/collision.cpp   75%
#   src/math/aatriangle.cpp       100%
#   src/math/random.cpp           100%
#   src/math/size.cpp            100%
#   src/math/sizef.cpp           100%
#   src/util/string_util.cpp       77%  (real 100%, under-reported by the
#                                      multi-binary .gcda overlap)
# Thresholds are set below these so the gate catches real regressions
# without false positives from that artifact.
DEFAULT_THRESHOLDS = {
    "src/collision/collision.cpp": 70.0,
    "src/math/aatriangle.cpp": 95.0,
    "src/math/random.cpp": 95.0,
    "src/math/size.cpp": 95.0,
    "src/math/sizef.cpp": 95.0,
    "src/util/string_util.cpp": 70.0,
}


def parse_overrides(pairs):
    out = {}
    for p in pairs:
        if ":" not in p:
            raise SystemExit(f"bad --threshold {p!r}, expected FILE:MIN")
        path, _, val = p.rpartition(":")
        try:
            out[path] = float(val)
        except ValueError:
            raise SystemExit(f"bad threshold value in {p!r}")
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                               formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("xml", help="gcovr --xml output file")
    ap.add_argument("--threshold", action="append", default=[],
                    help="FILE:MIN override (may repeat)")
    args = ap.parse_args()

    thresholds = dict(DEFAULT_THRESHOLDS)
    thresholds.update(parse_overrides(args.threshold))

    try:
        tree = ET.parse(args.xml)
    except FileNotFoundError:
        raise SystemExit(f"coverage xml not found: {args.xml}")

    root = tree.getroot()
    # gcovr XML: root <coverage>, children <package><class .../>
    classes = root.findall(".//class")
    if not classes:
        # also accept <file> layout
        classes = root.findall(".//file")

    # Map "filename" -> (line-rate as fraction, lines-valid, lines-covered)
    files = {}
    for c in classes:
        fn = c.get("filename")
        if not fn:
            continue
        lr = float(c.get("line-rate", "0"))
        lines_el = c.find("lines")
        valid = 0
        covered = 0
        if lines_el is not None:
            for ln in lines_el.findall("line"):
                valid += 1
                if int(ln.get("hits", "0")) > 0:
                    covered += 1
        files[fn] = (lr * 100.0, valid, covered)

    failures = []
    for path, minimum in sorted(thresholds.items()):
        # Match by suffix so absolute/relative paths both work.
        match = next((f for f in files if f.endswith(path)), None)
        if match is None:
            # Source was not compiled into any covered binary; treat as a
            # hard failure so a refactored build that drops the target is visible.
            failures.append(f"{path}: NOT FOUND in coverage report "
                           f"(expected >= {minimum:.0f}%)")
            continue
        pct, valid, covered = files[match]
        if valid == 0:
            continue  # no instrumented lines -> nothing to enforce
        if pct < minimum:
            failures.append(f"{path}: {pct:.0f}% < {minimum:.0f}% "
                           f"({covered}/{valid} lines)")
        else:
            print(f"OK  {path}: {pct:.0f}% (>= {minimum:.0f}%)")

    if failures:
        print("\nCOVERAGE GATE FAILED:")
        for f in failures:
            print(f"  - {f}")
        raise SystemExit(1)
    print("\nCOVERAGE GATE PASSED")


if __name__ == "__main__":
    main()
