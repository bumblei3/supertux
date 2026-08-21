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

// Coverage for math/vector.hpp: the SuperTux-specific polar/angle helpers
// that wrap glm::vec2. Vector itself is a glm::vec2 typedef (GLM is tested
// upstream), so we assert the SuperTux-owned free functions:
//   math::vec2_from_polar, math::angle, math::at_angle
// Header-only + glm, no engine libraries linked.

#include <gtest/gtest.h>

#include <cmath>

#include "math/vector.hpp"

namespace {

constexpr float PI = 3.14159265358979323846f;
constexpr float EPS = 1e-5f;

TEST(VectorTest, vec2_from_polar_basic)
{
  // length=1, angle=0  -> (1, 0)
  auto const v = math::vec2_from_polar(1.0f, 0.0f);
  EXPECT_NEAR(1.0f, v.x, EPS);
  EXPECT_NEAR(0.0f, v.y, EPS);

  // length=1, angle=pi/2 -> (0, 1)
  auto const w = math::vec2_from_polar(1.0f, PI / 2.0f);
  EXPECT_NEAR(0.0f, w.x, EPS);
  EXPECT_NEAR(1.0f, w.y, EPS);

  // length=2, angle=0 -> (2, 0)
  auto const u = math::vec2_from_polar(2.0f, 0.0f);
  EXPECT_NEAR(2.0f, u.x, EPS);
  EXPECT_NEAR(0.0f, u.y, EPS);
}

TEST(VectorTest, angle_basic)
{
  // Positive x-axis is angle 0
  EXPECT_NEAR(0.0f, math::angle(Vector(1.0f, 0.0f)), EPS);
  // Positive y-axis is angle pi/2
  EXPECT_NEAR(PI / 2.0f, math::angle(Vector(0.0f, 1.0f)), EPS);
  // Negative x-axis is angle pi
  EXPECT_NEAR(PI, math::angle(Vector(-1.0f, 0.0f)), EPS);
  // 45 degrees
  EXPECT_NEAR(PI / 4.0f, math::angle(Vector(1.0f, 1.0f)), EPS);
}

TEST(VectorTest, angle_zero_vector_is_zero)
{
  // Edge case: the null vector must return 0, not NaN from atan2(0,0).
  EXPECT_NEAR(0.0f, math::angle(Vector(0.0f, 0.0f)), EPS);
}

TEST(VectorTest, polar_roundtrip)
{
  // Converting to polar and back must recover the original vector.
  Vector const v(3.0f, -4.0f);
  float const len = std::sqrt(v.x * v.x + v.y * v.y);
  float const ang = math::angle(v);
  auto const back = math::vec2_from_polar(len, ang);
  EXPECT_NEAR(v.x, back.x, EPS);
  EXPECT_NEAR(v.y, back.y, EPS);
}

TEST(VectorTest, at_angle_preserves_length_changes_direction)
{
  Vector const v(3.0f, 4.0f);  // length 5
  auto const a = math::at_angle(v, 0.0f);
  // length is preserved (5), direction is now along +x
  EXPECT_NEAR(5.0f, std::sqrt(a.x * a.x + a.y * a.y), EPS);
  EXPECT_NEAR(5.0f, a.x, EPS);
  EXPECT_NEAR(0.0f, a.y, EPS);
}

} // namespace
