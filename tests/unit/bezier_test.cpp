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

#include "st_assert.hpp"
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

int main(void)
{
  const Vector p1(0.0f, 0.0f);
  const Vector p2(0.0f, 10.0f);
  const Vector p3(10.0f, 10.0f);
  const Vector p4(10.0f, 0.0f);

  // Endpoints: t=0 yields p1, t=1 yields p4 (both get_point and _raw).
  {
    ST_ASSERT("get_point t=0 -> p1", vapprox(Bezier::get_point(p1, p2, p3, p4, 0.0f), p1));
    ST_ASSERT("get_point t=1 -> p4", vapprox(Bezier::get_point(p1, p2, p3, p4, 1.0f), p4));
    ST_ASSERT("get_point_raw t=0 -> p1", vapprox(Bezier::get_point_raw(p1, p2, p3, p4, 0.0f), p1));
    ST_ASSERT("get_point_raw t=1 -> p4", vapprox(Bezier::get_point_raw(p1, p2, p3, p4, 1.0f), p4));
  }

  // Degenerate "linear" control net (p1==p2, p3==p4): the curve is the straight
  // segment p1->p4, so t=0.5 lands exactly on the midpoint.
  {
    Vector a(0.0f, 0.0f), b(0.0f, 0.0f), c(10.0f, 20.0f), d(10.0f, 20.0f);
    Vector mid = Bezier::get_point(a, b, c, d, 0.5f);
    ST_ASSERT("linear net: midpoint", vapprox(mid, Vector(5.0f, 10.0f)));
  }

  // Symmetric control net about x=5: the midpoint (t=0.5) must have x=5.
  {
    Vector mid = Bezier::get_point(p1, p2, p3, p4, 0.5f);
    ST_ASSERT("symmetric net: midpoint x = 5", approx(mid.x, 5.0f));
    // The curve bulges upward; midpoint y should be positive and below the
    // handle height of 10.
    ST_ASSERT("symmetric net: 0 < midpoint y < 10", mid.y > 0.0f && mid.y < 10.0f);
  }

  // get_length of a straight linear net equals the Euclidean distance.
  {
    Vector a(0.0f, 0.0f), b(0.0f, 0.0f), c(30.0f, 40.0f), d(30.0f, 40.0f);
    float len = Bezier::get_length(a, b, c, d, 100);
    ST_ASSERT("linear length = 50 (3-4-5)", approx(len, 50.0f, 0.01f));
  }

  // get_point_at_length(0) returns p1; the full length returns ~p4.
  {
    Vector a(0.0f, 0.0f), b(0.0f, 0.0f), c(30.0f, 40.0f), d(30.0f, 40.0f);
    ST_ASSERT("point_at_length 0 -> p1",
              vapprox(Bezier::get_point_at_length(a, b, c, d, 0.0f), a));
    float full = Bezier::get_length(a, b, c, d);
    ST_ASSERT("point_at_length(full) -> ~p4",
              vapprox(Bezier::get_point_at_length(a, b, c, d, full), d, 0.1f));
    // Half the length on a straight line is the geometric midpoint.
    ST_ASSERT("point_at_length(half) -> midpoint",
              vapprox(Bezier::get_point_at_length(a, b, c, d, full * 0.5f),
                      Vector(15.0f, 20.0f), 0.1f));
  }

  // get_point_by_length(t) is length-normalized; t=0 -> p1, t=1 -> ~p4.
  {
    ST_ASSERT("point_by_length t=0 -> p1",
              vapprox(Bezier::get_point_by_length(p1, p2, p3, p4, 0.0f), p1));
    ST_ASSERT("point_by_length t=1 -> ~p4",
              vapprox(Bezier::get_point_by_length(p1, p2, p3, p4, 1.0f), p4, 0.2f));
  }

  // Curve length is at least the straight-line distance between the anchors.
  {
    float len = Bezier::get_length(p1, p2, p3, p4);
    float chord = std::sqrt((p4.x - p1.x) * (p4.x - p1.x) +
                            (p4.y - p1.y) * (p4.y - p1.y));
    ST_ASSERT("length >= chord", len >= chord - 0.001f);
  }
}

/* EOF */
