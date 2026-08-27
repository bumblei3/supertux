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
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Coverage for math/anchor_point.hpp: the 9 anchor positions over a known
// Rectf, placed-object offsets, quarter-offset centres, and string<->anchor
// round-trips. tinygettext/logging stubbed (anchor_point_test_stub.cpp); the
// test is linked with tinygettext so gettext.hpp can pull in its header.
//
// Converted to GoogleTest for better failure diagnostics.

#include <gtest/gtest.h>

#include "math/rectf.hpp"
#include "math/anchor_point.hpp"

#include <stdexcept>
#include <string>

// A 100x40 rect anchored at (10, 20): left=10 right=110 top=20 bottom=60,
// horizontal middle = 60, vertical middle = 40.

TEST(AnchorPointTest, get_anchor_pos_all_9_corners)
{
  Rectf rect(10.0f, 20.0f, 110.0f, 60.0f);

  EXPECT_NEAR(get_anchor_pos(rect, ANCHOR_TOP_LEFT).x, 10.0f, 0.001f);
  EXPECT_NEAR(get_anchor_pos(rect, ANCHOR_TOP_LEFT).y, 20.0f, 0.001f);

  EXPECT_NEAR(get_anchor_pos(rect, ANCHOR_TOP).x, 60.0f, 0.001f);
  EXPECT_NEAR(get_anchor_pos(rect, ANCHOR_TOP).y, 20.0f, 0.001f);

  EXPECT_NEAR(get_anchor_pos(rect, ANCHOR_TOP_RIGHT).x, 110.0f, 0.001f);
  EXPECT_NEAR(get_anchor_pos(rect, ANCHOR_TOP_RIGHT).y, 20.0f, 0.001f);

  EXPECT_NEAR(get_anchor_pos(rect, ANCHOR_MIDDLE).x, 60.0f, 0.001f);
  EXPECT_NEAR(get_anchor_pos(rect, ANCHOR_MIDDLE).y, 40.0f, 0.001f);

  EXPECT_NEAR(get_anchor_pos(rect, ANCHOR_BOTTOM_RIGHT).x, 110.0f, 0.001f);
  EXPECT_NEAR(get_anchor_pos(rect, ANCHOR_BOTTOM_RIGHT).y, 60.0f, 0.001f);

  EXPECT_NEAR(get_anchor_pos(rect, ANCHOR_BOTTOM_LEFT).x, 10.0f, 0.001f);
  EXPECT_NEAR(get_anchor_pos(rect, ANCHOR_BOTTOM_LEFT).y, 60.0f, 0.001f);
}

TEST(AnchorPointTest, get_anchor_pos_with_placed_object)
{
  // For a 20x10 object anchored bottom-right, the top-left corner sits at
  // (right - width, bottom - height) = (110-20, 60-10) = (90, 50).
  Rectf rect(10.0f, 20.0f, 110.0f, 60.0f);

  EXPECT_NEAR(get_anchor_pos(rect, 20.0f, 10.0f, ANCHOR_BOTTOM_RIGHT).x, 90.0f, 0.001f);
  EXPECT_NEAR(get_anchor_pos(rect, 20.0f, 10.0f, ANCHOR_BOTTOM_RIGHT).y, 50.0f, 0.001f);

  EXPECT_NEAR(get_anchor_pos(rect, 20.0f, 10.0f, ANCHOR_MIDDLE).x, 60.0f - 10.0f, 0.001f);
  EXPECT_NEAR(get_anchor_pos(rect, 20.0f, 10.0f, ANCHOR_MIDDLE).y, 40.0f - 5.0f, 0.001f);

  EXPECT_NEAR(get_anchor_pos(rect, 20.0f, 10.0f, ANCHOR_TOP_LEFT).x, 10.0f, 0.001f);
  EXPECT_NEAR(get_anchor_pos(rect, 20.0f, 10.0f, ANCHOR_TOP_LEFT).y, 20.0f, 0.001f);
}

TEST(AnchorPointTest, get_anchor_center_pos_quarter_offsets)
{
  // Left column -> left + width/4 = 10 + 25 = 35.
  Rectf rect(10.0f, 20.0f, 110.0f, 60.0f);

  EXPECT_NEAR(get_anchor_center_pos(rect, ANCHOR_TOP_LEFT).x, 35.0f, 0.001f);
  EXPECT_NEAR(get_anchor_center_pos(rect, ANCHOR_TOP_LEFT).y, 30.0f, 0.001f);

  EXPECT_NEAR(get_anchor_center_pos(rect, ANCHOR_MIDDLE).x, 60.0f, 0.001f);
  EXPECT_NEAR(get_anchor_center_pos(rect, ANCHOR_MIDDLE).y, 40.0f, 0.001f);
}

TEST(AnchorPointTest, string_anchor_roundtrip_all_9)
{
  for (int i = 0; i <= ANCHOR_LAST; ++i)
  {
    AnchorPoint ap = static_cast<AnchorPoint>(i);
    std::string s = anchor_point_to_string(ap);
    AnchorPoint back = string_to_anchor_point(s);
    EXPECT_EQ(back, ap) << "AnchorPoint " << i << " round-trip failed";
  }
}

TEST(AnchorPointTest, known_string_mappings)
{
  EXPECT_EQ(string_to_anchor_point("topleft"), ANCHOR_TOP_LEFT);
  EXPECT_EQ(string_to_anchor_point("bottomright"), ANCHOR_BOTTOM_RIGHT);
  EXPECT_EQ(anchor_point_to_string(ANCHOR_MIDDLE), "middle");
}

TEST(AnchorPointTest, unknown_string_throws)
{
  EXPECT_THROW(string_to_anchor_point("nonsense"), std::exception);
}