//  SuperTux
//  Copyright (C) 2026 Tobias Berner <tobias.berner@mailbox.org>
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
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Regression coverage for math/aatriangle.cpp :: AATriangle::vertical_flip.
// Previously 0% covered in the fork suite.

#include <gtest/gtest.h>

#include "math/aatriangle.hpp"

namespace {

using Dir = AATriangle::Direction;

TEST(AATriangleVerticalFlipTest, direction_inverts_around_3)
{
  // vertical_flip maps direction d -> (3 - d), i.e. it flips the vertical
  // orientation of the slope (bottom-facing <-> top-facing).
  EXPECT_EQ(AATriangle::vertical_flip(Dir::SOUTHWEST), Dir::NORTHWEST); // 0 -> 3
  EXPECT_EQ(AATriangle::vertical_flip(Dir::NORTHEAST), Dir::SOUTHEAST); // 1 -> 2
  EXPECT_EQ(AATriangle::vertical_flip(Dir::SOUTHEAST), Dir::NORTHEAST); // 2 -> 1
  EXPECT_EQ(AATriangle::vertical_flip(Dir::NORTHWEST), Dir::SOUTHWEST); // 3 -> 0
}

TEST(AATriangleVerticalFlipTest, direction_is_its_own_inverse)
{
  // Flipping twice returns the original direction.
  for (int d = 0; d < 4; ++d) {
    SCOPED_TRACE(d);
    EXPECT_EQ(AATriangle::vertical_flip(AATriangle::vertical_flip(d)), d);
  }
}

TEST(AATriangleVerticalFlipTest, deform_bottom_top_swap)
{
  // DEFORM_BOTTOM <-> DEFORM_TOP, while DEFORM_LEFT/DEFORM_RIGHT are
  // untouched (they are horizontal deformations).
  EXPECT_EQ(AATriangle::vertical_flip(Dir::SOUTHWEST | Dir::DEFORM_BOTTOM),
            (Dir::NORTHWEST | Dir::DEFORM_TOP));   // 0x10 -> 0x20
  EXPECT_EQ(AATriangle::vertical_flip(Dir::SOUTHWEST | Dir::DEFORM_TOP),
            (Dir::NORTHWEST | Dir::DEFORM_BOTTOM)); // 0x20 -> 0x10
}

TEST(AATriangleVerticalFlipTest, deform_left_right_unchanged)
{
  EXPECT_EQ(AATriangle::vertical_flip(Dir::SOUTHWEST | Dir::DEFORM_LEFT),
            (Dir::NORTHWEST | Dir::DEFORM_LEFT));   // 0x30 unchanged
  EXPECT_EQ(AATriangle::vertical_flip(Dir::SOUTHWEST | Dir::DEFORM_RIGHT),
            (Dir::NORTHWEST | Dir::DEFORM_RIGHT));  // 0x40 unchanged
}

TEST(AATriangleVerticalFlipTest, flip_preserves_deform_mask_only)
{
  // Only the vertical deform flags swap; the horizontal ones stay put and the
  // direction is always inverted to its 3-d complement.
  const int base = Dir::SOUTHEAST | Dir::DEFORM_LEFT;
  const int flipped = AATriangle::vertical_flip(base);
  EXPECT_EQ(flipped & Dir::DIRECTION_MASK, static_cast<int>(Dir::NORTHEAST));
  EXPECT_EQ(flipped & Dir::DEFORM_MASK, static_cast<int>(Dir::DEFORM_LEFT));
}

} // namespace

/* EOF */
