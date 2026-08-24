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

// Coverage for math/util.hpp: clamp, sgn, in_bounds, positive_mod,
// positive_fmodf, degrees/radians. Header-only, engine-free.

#include <gtest/gtest.h>

#include <limits>

#include "math/util.hpp"

namespace {

constexpr float EPS = 1e-6f;

TEST(MathUtilTest, clamp_returns_value_in_range)
{
  EXPECT_FLOAT_EQ(0.5f, math::clamp(0.5f, 0.0f, 1.0f));
  EXPECT_FLOAT_EQ(0.0f, math::clamp(-1.0f, 0.0f, 1.0f));
  EXPECT_FLOAT_EQ(1.0f, math::clamp(2.0f, 0.0f, 1.0f));
}

TEST(MathUtilTest, clamp_boundaries_are_inclusive)
{
  EXPECT_FLOAT_EQ(0.0f, math::clamp(0.0f, 0.0f, 1.0f));
  EXPECT_FLOAT_EQ(1.0f, math::clamp(1.0f, 0.0f, 1.0f));
}

TEST(MathUtilTest, clamp_int)
{
  EXPECT_EQ(5, math::clamp(5, 0, 10));
  EXPECT_EQ(0, math::clamp(-3, 0, 10));
  EXPECT_EQ(10, math::clamp(99, 0, 10));
}

TEST(MathUtilTest, sgn)
{
  EXPECT_EQ(1, math::sgn(42));
  EXPECT_EQ(-1, math::sgn(-42));
  EXPECT_EQ(0, math::sgn(0));

  EXPECT_EQ(1, math::sgn(0.5f));
  EXPECT_EQ(-1, math::sgn(-0.001f));
  EXPECT_EQ(0, math::sgn(0.0f));
}

TEST(MathUtilTest, in_bounds_inclusive)
{
  EXPECT_TRUE(math::in_bounds(5, 0, 10));
  EXPECT_TRUE(math::in_bounds(0, 0, 10));   // lower boundary inclusive
  EXPECT_TRUE(math::in_bounds(10, 0, 10));  // upper boundary inclusive
  EXPECT_FALSE(math::in_bounds(-1, 0, 10));
  EXPECT_FALSE(math::in_bounds(11, 0, 10));
}

TEST(MathUtilTest, positive_mod_basic)
{
  EXPECT_EQ(3, math::positive_mod(7, 4));
  EXPECT_EQ(0, math::positive_mod(8, 4));
}

TEST(MathUtilTest, positive_mod_negative_lhs_is_wrapped_positive)
{
  // The whole point vs plain %: result is always in [0, rhs).
  EXPECT_EQ(1, math::positive_mod(-7, 4));
  EXPECT_EQ(0, math::positive_mod(-4, 4));
  EXPECT_EQ(2, math::positive_mod(-1, 3));
}

TEST(MathUtilTest, positive_fmodf_negative_lhs_is_wrapped_positive)
{
  float const r = math::positive_fmodf(-1.0f, 4.0f);
  EXPECT_GE(r, 0.0f);
  EXPECT_LT(r, 4.0f);
  EXPECT_NEAR(3.0f, r, EPS);

  EXPECT_NEAR(1.5f, math::positive_fmodf(1.5f, 4.0f), EPS);
}

TEST(MathUtilTest, degrees_radians_roundtrip)
{
  EXPECT_NEAR(180.0f, math::degrees(math::PI), EPS);
  EXPECT_NEAR(math::PI, math::radians(180.0f), EPS);
  EXPECT_NEAR(math::TAU, math::radians(360.0f), EPS);

  // Roundtrip over an arbitrary angle.
  float const angle = 123.456f;
  EXPECT_NEAR(angle, math::degrees(math::radians(angle)), 1e-3f);
}

TEST(MathUtilTest, constants_consistent)
{
  EXPECT_NEAR(2.0f * math::PI, math::TAU, EPS);
  EXPECT_NEAR(math::PI / 2.0f, math::PI_2, EPS);
  EXPECT_NEAR(math::PI / 4.0f, math::PI_4, EPS);
}

} // namespace

/* EOF */
