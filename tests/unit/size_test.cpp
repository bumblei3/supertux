//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "math/size.hpp"
#include "math/sizef.hpp"

TEST(SizeTest, constructors_and_equality)
{
  Size def;
  EXPECT_EQ(def.width, 0);
  EXPECT_EQ(def.height, 0);

  Size s(800, 600);
  EXPECT_EQ(Size(800, 600), s);
  EXPECT_FALSE(Size(800, 600) != s);
  EXPECT_TRUE(Size(800, 600) != Size(801, 600));
}

TEST(SizeTest, arithmetic_operators)
{
  Size s(800, 600);
  EXPECT_EQ(Size(1600, 1200), s * 2);
  EXPECT_EQ(Size(1600, 1200), 2 * s);
  EXPECT_EQ(Size(400, 300), s / 2);
  EXPECT_EQ(Size(1000, 900), s + Size(200, 300));
  EXPECT_EQ(Size(600, 300), s - Size(200, 300));
}

TEST(SizeTest, compound_assignment)
{
  Size s(800, 600);
  s *= 2;
  EXPECT_EQ(Size(1600, 1200), s);
  s /= 2;
  EXPECT_EQ(Size(800, 600), s);
  s += Size(100, 50);
  EXPECT_EQ(Size(900, 650), s);
  s -= Size(100, 50);
  EXPECT_EQ(Size(800, 600), s);
}

TEST(SizeTest, is_valid)
{
  EXPECT_TRUE(Size(800, 600).is_valid());
  EXPECT_FALSE(Size(0, 600).is_valid());
  EXPECT_FALSE(Size(800, 0).is_valid());
  EXPECT_FALSE(Size(-1, 600).is_valid());
  EXPECT_FALSE(Size(0, 0).is_valid());
}

TEST(SizeTest, conversion_from_sizef_truncates)
{
  // Width/height are static_cast<int>, so fractional parts are dropped.
  Size s(Sizef(800.9f, 600.4f));
  EXPECT_EQ(Size(800, 600), s);
}

TEST(SizeTest, stream_output)
{
  std::ostringstream os;
  os << Size(800, 600);
  EXPECT_EQ(os.str(), "Size(800, 600)");
}

/* EOF */
