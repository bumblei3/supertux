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

#include "math/sizef.hpp"
#include "math/size.hpp"
#include "math/vector.hpp"

TEST(SizefTest, constructors_and_equality)
{
  Sizef def;
  EXPECT_FLOAT_EQ(def.width, 0.0f);
  EXPECT_FLOAT_EQ(def.height, 0.0f);

  Sizef s(800.0f, 600.0f);
  EXPECT_EQ(Sizef(800.0f, 600.0f), s);
  EXPECT_FALSE(Sizef(800.0f, 600.0f) != s);
  EXPECT_TRUE(Sizef(800.0f, 600.0f) != Sizef(801.0f, 600.0f));
}

TEST(SizefTest, from_vector)
{
  Sizef s(Vector(12.5f, 3.25f));
  EXPECT_FLOAT_EQ(s.width, 12.5f);
  EXPECT_FLOAT_EQ(s.height, 3.25f);
}

TEST(SizefTest, arithmetic_operators)
{
  Sizef s(800.0f, 600.0f);
  EXPECT_EQ(Sizef(1600.0f, 1200.0f), s * 2.0f);
  EXPECT_EQ(Sizef(1600.0f, 1200.0f), 2.0f * s);
  EXPECT_EQ(Sizef(400.0f, 300.0f), s / 2.0f);
  EXPECT_EQ(Sizef(1000.0f, 900.0f), s + Sizef(200.0f, 300.0f));
  EXPECT_EQ(Sizef(600.0f, 300.0f), s - Sizef(200.0f, 300.0f));
}

TEST(SizefTest, compound_assignment)
{
  Sizef s(800.0f, 600.0f);
  s *= 2.0f;
  EXPECT_EQ(Sizef(1600.0f, 1200.0f), s);
  s /= 2.0f;
  EXPECT_EQ(Sizef(800.0f, 600.0f), s);
  s += Sizef(100.0f, 50.0f);
  EXPECT_EQ(Sizef(900.0f, 650.0f), s);
  s -= Sizef(100.0f, 50.0f);
  EXPECT_EQ(Sizef(800.0f, 600.0f), s);
}

TEST(SizefTest, as_vector)
{
  Sizef s(12.5f, 3.25f);
  Vector v = s.as_vector();
  EXPECT_FLOAT_EQ(v.x, 12.5f);
  EXPECT_FLOAT_EQ(v.y, 3.25f);
}

TEST(SizefTest, is_valid)
{
  EXPECT_TRUE(Sizef(800.0f, 600.0f).is_valid());
  EXPECT_FALSE(Sizef(0.0f, 600.0f).is_valid());
  EXPECT_FALSE(Sizef(800.0f, 0.0f).is_valid());
  EXPECT_FALSE(Sizef(-1.0f, 600.0f).is_valid());
}

TEST(SizefTest, conversion_from_size)
{
  Sizef s(Size(800, 600));
  EXPECT_FLOAT_EQ(s.width, 800.0f);
  EXPECT_FLOAT_EQ(s.height, 600.0f);
}

TEST(SizefTest, stream_output)
{
  std::ostringstream os;
  os << Sizef(800.0f, 600.0f);
  EXPECT_EQ(os.str(), "Sizef(800, 600)");
}

/* EOF */
