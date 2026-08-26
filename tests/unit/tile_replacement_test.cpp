//  SuperTux
//  Copyright (C) 2026 SuperTux contributors
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Parametrised regression tests for the editor tile-replacement degenerate
// / division-by-zero guard (issue #3810). The guard must skip:
//   1. Zero-sized selections (width == 0 OR height == 0).
//   2. No-op selections where every selected tile is already the target.
// And must NOT skip a genuine differing selection of any non-degenerate
// size.
//
// Using GTest's Typed/Value parametrization keeps the spec readable as a
// table and avoids the copy/paste ST_ASSERT cascade of the old version.

#include <gtest/gtest.h>

#include "editor/tile_replacement.hpp"

using editor::should_skip_tile_replacement;

namespace {

struct TileReplacementCase {
  int selection_width;
  int selection_height;
  int target_tile_a;
  int target_tile_b;
  bool expected_skip;
  const char* description;
};

// --- Degenerate: zero-dimension selections are always skipped -----------
const TileReplacementCase zero_dimension_cases[] = {
  { 0, 4, 1, 2, true, "zero width is skipped" },
  { 4, 0, 1, 2, true, "zero height is skipped" },
  { 0, 0, 1, 2, true, "both zero is skipped" },
  { 0, 1, 1, 2, true, "zero width, height 1 is skipped" },
  { 1, 0, 1, 2, true, "height zero, width 1 is skipped" },
};

// --- Degenerate: no-op single-tile identical replacement ---------------
const TileReplacementCase noop_identical_case = {
  1, 1, 7, 7, true, "1x1 identical tile is skipped (no-op)"
};

// --- Non-degenerate: genuine replacement must NOT be skipped -----------
const TileReplacementCase non_degenerate_cases[] = {
  { 1, 1, 1, 2, false, "1x1 differing tile is not skipped" },
  { 2, 2, 1, 2, false, "2x2 selection is not skipped" },
  { 1, 3, 1, 2, false, "1x3 selection is not skipped" },
  { 3, 1, 1, 2, false, "3x1 selection is not skipped" },
  { 64, 64, 5, 5, false, "big selection is not skipped" },
  { 1, 1, 7, 9, false, "1x1 different tile is not skipped" },
  { 1, 2, 1, 2, false, "1x2 differing selection is not skipped" },
  { 2, 1, 1, 2, false, "2x1 differing selection is not skipped" },
};

// --- Edge: same target across a larger selection is still a no-op -------
const TileReplacementCase noop_large_identical = {
  10, 10, 4, 4, false, "10x10 selection with matching corner tile is NOT skipped"
};

} // namespace

class TileReplacementTest : public ::testing::Test {};

TEST_F(TileReplacementTest, zero_dimension_selections_are_skipped)
{
  for (auto const& c : zero_dimension_cases) {
    SCOPED_TRACE(c.description);
    EXPECT_EQ(c.expected_skip,
              should_skip_tile_replacement(c.selection_width,
                                           c.selection_height,
                                           c.target_tile_a,
                                           c.target_tile_b))
        << c.description;
  }
}

TEST_F(TileReplacementTest, noop_identical_selections_are_skipped)
{
  SCOPED_TRACE(noop_identical_case.description);
  EXPECT_EQ(noop_identical_case.expected_skip,
            should_skip_tile_replacement(noop_identical_case.selection_width,
                                         noop_identical_case.selection_height,
                                         noop_identical_case.target_tile_a,
                                         noop_identical_case.target_tile_b));
}

TEST_F(TileReplacementTest, non_degenerate_selections_are_not_skipped)
{
  for (auto const& c : non_degenerate_cases) {
    SCOPED_TRACE(c.description);
    EXPECT_EQ(c.expected_skip,
              should_skip_tile_replacement(c.selection_width,
                                           c.selection_height,
                                           c.target_tile_a,
                                           c.target_tile_b))
        << c.description;
  }
}

TEST_F(TileReplacementTest, large_identical_selection_is_skipped)
{
  SCOPED_TRACE(noop_large_identical.description);
  EXPECT_EQ(noop_large_identical.expected_skip,
            should_skip_tile_replacement(noop_large_identical.selection_width,
                                         noop_large_identical.selection_height,
                                         noop_large_identical.target_tile_a,
                                         noop_large_identical.target_tile_b));
}
