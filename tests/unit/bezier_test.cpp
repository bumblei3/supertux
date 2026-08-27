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

// Coverage for math/bezier.hpp cubic-Bezier math (the draw_curve() rendering
// path is intentionally not exercised). Verifies endpoint interpolation, the
// collinear/linear case, midpoint symmetry, arc-length approximation, and
// get_point_at_length. Logging stubbed (bezier_test_stub.cpp).

#include <gtest/gtest.h>
#include "math/bezier.hpp"
#include "math/vector.hpp"

#include <cmath>

namespace {

bool approx(float a, float b, float eps = 0.001f)
{
  float d = a - b;
  return (d < 0 ? -d : d) < eps;
}

bool vapprox(const Vector& a, const Vector& b, float eps = 0.01f)
{
  return approx(a.x, b.x, eps) && approx(a.y, b.y, eps);
}

} // namespace

TEST(BezierTest, endpoints_t0_t1)
{
  const Vector p1(0.0f, 0.0f);
  const Vector p2(0.0f, 10.0f);
  const Vector p3(10.0f, 10.0f);
  const Vector p4(10.0f, 0.0f);

  EXPECT_TRUE(vapprox(Bezier::get_point(p1, p2, p3, p4, 0.0f), p1));
  EXPECT_TRUE(vapprox(Bezier::get_point(p1, p2, p3, p4, 1.0f), p4));
  EXPECT_TRUE(vapprox(Bezier::get_point_raw(p1, p2, p3, p4, 0.0f), p1));
  EXPECT_TRUE(vapprox(Bezier::get_point_raw(p1, p2, p3, p4, 1.0f), p4));
}

TEST(BezierTest, degenerate_linear_net_midpoint)
{
  // Collinear control net where p1==p2 and p3==p4: the curve is the straight
  // segment p1->p4, so t=0.5 lands exactly on the midpoint.
  Vector a(0.0f, 0.0f), b(0.0f, 0.0f), c(10.0f, 20.0f), d(10.0f, 20.0f);
  Vector mid = Bezier::get_point(a, b, c, d, 0.5f);
  EXPECT_TRUE(vapprox(mid, Vector(5.0f, 10.0f)));
}

TEST(BezierTest, symmetric_control_net_midpoint)
{
  // Symmetric control net about x=5: the midpoint (t=0.5) must have x=5.
  Vector p1(0.0f, 0.0f), p2(0.0f, 10.0f), p3(10.0f, 10.0f), p4(10.0f, 0.0f);
  Vector mid = Bezier::get_point(p1, p2, p3, p4, 0.5f);
  EXPECT_NEAR(mid.x, 5.0f, 0.001f);
  EXPECT_GT(mid.y, 0.0f);
  EXPECT_LT(mid.y, 10.0f);
}

TEST(BezierTest, linear_net_length)
{
  Vector a(0.0f, 0.0f), b(0.0f, 0.0f), c(30.0f, 40.0f), d(30.0f, 40.0f);
  float len = Bezier::get_length(a, b, c, d, 100);
  EXPECT_NEAR(len, 50.0f, 0.01f);
}

TEST(BezierTest, get_point_at_length_bounds_and_midpoint)
{
  Vector a(0.0f, 0.0f), b(0.0f, 0.0f), c(30.0f, 40.0f), d(30.0f, 40.0f);
  EXPECT_TRUE(vapprox(Bezier::get_point_at_length(a, b, c, d, 0.0f), a));
  float full = Bezier::get_length(a, b, c, d);
  EXPECT_TRUE(vapprox(Bezier::get_point_at_length(a, b, c, d, full), d, 0.1f));
  EXPECT_TRUE(vapprox(Bezier::get_point_at_length(a, b, c, d, full * 0.5f),
                      Vector(15.0f, 20.0f), 0.1f));
}

TEST(BezierTest, get_point_by_length_normalization)
{
  Vector p1(0.0f, 0.0f), p2(0.0f, 10.0f), p3(10.0f, 10.0f), p4(10.0f, 0.0f);
  EXPECT_TRUE(vapprox(Bezier::get_point_by_length(p1, p2, p3, p4, 0.0f), p1));
  EXPECT_TRUE(vapprox(Bezier::get_point_by_length(p1, p2, p3, p4, 1.0f), p4, 0.2f));
}

TEST(BezierTest, curve_length_at_least_chord)
{
  Vector p1(0.0f, 0.0f), p2(0.0f, 10.0f), p3(10.0f, 10.0f), p4(10.0f, 0.0f);
  float len = Bezier::get_length(p1, p2, p3, p4);
  float chord = std::sqrt((p4.x - p1.x) * (p4.x - p1.x) +
                           (p4.y - p1.y) * (p4.y - p1.y));
  EXPECT_GE(len, chord - 0.001f);
}

/* EOF */
