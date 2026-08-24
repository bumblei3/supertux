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

// Coverage for supertux/direction.cpp: dir_to_string, string_to_dir,
// invert_dir, operator<<. Engine-free linkable (log stub + gettext +
// color.cpp + tinygettext), see direction_test_stub.cpp.

#include <gtest/gtest.h>

#include <sstream>

#include "supertux/direction.hpp"

TEST(DirectionTest, dir_to_string_all_directions)
{
  EXPECT_EQ("auto", dir_to_string(Direction::AUTO));
  EXPECT_EQ("none", dir_to_string(Direction::NONE));
  EXPECT_EQ("left", dir_to_string(Direction::LEFT));
  EXPECT_EQ("right", dir_to_string(Direction::RIGHT));
  EXPECT_EQ("up", dir_to_string(Direction::UP));
  EXPECT_EQ("down", dir_to_string(Direction::DOWN));
}

TEST(DirectionTest, string_to_dir_all_directions)
{
  EXPECT_EQ(Direction::NONE, string_to_dir("none"));
  EXPECT_EQ(Direction::LEFT, string_to_dir("left"));
  EXPECT_EQ(Direction::RIGHT, string_to_dir("right"));
  EXPECT_EQ(Direction::UP, string_to_dir("up"));
  EXPECT_EQ(Direction::DOWN, string_to_dir("down"));
}

TEST(DirectionTest, string_to_dir_unknown_falls_back_to_auto)
{
  // Unknown strings must degrade to AUTO, never crash or UB.
  EXPECT_EQ(Direction::AUTO, string_to_dir(""));
  EXPECT_EQ(Direction::AUTO, string_to_dir("Left"));   // case-sensitive parser
  EXPECT_EQ(Direction::AUTO, string_to_dir("diagonal"));
}

TEST(DirectionTest, string_roundtrip)
{
  for (Direction dir : { Direction::AUTO, Direction::NONE,
                         Direction::LEFT, Direction::RIGHT,
                         Direction::UP, Direction::DOWN })
  {
    EXPECT_EQ(dir, string_to_dir(dir_to_string(dir)));
  }
}

TEST(DirectionTest, invert_dir_horizontal_and_vertical)
{
  EXPECT_EQ(Direction::RIGHT, invert_dir(Direction::LEFT));
  EXPECT_EQ(Direction::LEFT, invert_dir(Direction::RIGHT));
  EXPECT_EQ(Direction::UP, invert_dir(Direction::DOWN));
  EXPECT_EQ(Direction::DOWN, invert_dir(Direction::UP));
}

TEST(DirectionTest, invert_dir_degenerate_returns_none)
{
  // AUTO and NONE have no inverse -> NONE.
  EXPECT_EQ(Direction::NONE, invert_dir(Direction::AUTO));
  EXPECT_EQ(Direction::NONE, invert_dir(Direction::NONE));
}

TEST(DirectionTest, double_inversion_is_identity)
{
  for (Direction dir : { Direction::LEFT, Direction::RIGHT,
                         Direction::UP, Direction::DOWN })
  {
    EXPECT_EQ(dir, invert_dir(invert_dir(dir)));
  }
}

TEST(DirectionTest, stream_operator_uses_dir_to_string)
{
  std::ostringstream out;
  out << Direction::LEFT;
  EXPECT_EQ("left", out.str());
}

/* EOF */
