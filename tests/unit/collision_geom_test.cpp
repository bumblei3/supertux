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

// Coverage for collision/collision.hpp pure geometry: line_intersects_line,
// intersects_line (Rect vs line segment), and rectangle_aatriangle. These are
// exercised by the engine but not asserted by the existing gtest CollisionTest.
// Logging stubbed (collision_test_stub.cpp) to stay engine-free.

#include "st_assert.hpp"
#include "collision/collision.hpp"
#include "math/aatriangle.hpp"
#include "math/rectf.hpp"

namespace {

bool approx(float a, float b, float eps = 0.001f)
{
  float d = a - b;
  return (d < 0 ? -d : d) < eps;
}

} // namespace

int main(void)
{
  // --- line_intersects_line -------------------------------------------------

  // Crossing X: one horizontal line y=0 across x in [-1,1], one vertical
  // line x=0 across y in [-1,1]: they intersect at the origin.
  {
    ST_ASSERT("crossing lines intersect",
              collision::line_intersects_line(Vector(-1, 0), Vector(1, 0),
                                               Vector(0, -1), Vector(0, 1)));
  }

  // Parallel lines (both horizontal at different y) never intersect.
  {
    ST_ASSERT("parallel lines do not intersect",
              !collision::line_intersects_line(Vector(-1, 0), Vector(1, 0),
                                                Vector(-1, 1), Vector(1, 1)));
  }

  // Collinear overlapping lines DO intersect (share infinitely many points).
  {
    ST_ASSERT("collinear overlapping lines intersect",
              collision::line_intersects_line(Vector(-2, 0), Vector(0, 0),
                                               Vector(0, 0), Vector(2, 0)));
  }

  // Touching at an endpoint only (T junction): the segment endpoint lies on
  // the other segment -> considered intersecting.
  {
    ST_ASSERT("endpoint-touching lines intersect",
              collision::line_intersects_line(Vector(0, 0), Vector(2, 0),
                                               Vector(0, 0), Vector(0, 2)));
  }

  // Disjoint non-parallel lines (cross but outside both segments) do not.
  {
    ST_ASSERT("non-crossing non-parallel lines do not intersect",
              !collision::line_intersects_line(Vector(0, 0), Vector(1, 0),
                                                Vector(2, 2), Vector(3, 3)));
  }

  // --- intersects_line (Rect vs segment) ------------------------------------

  Rectf rect(0.0f, 0.0f, 10.0f, 10.0f);

  // A line passing through the middle of the rect intersects it.
  {
    ST_ASSERT("segment through rect intersects",
              collision::intersects_line(rect, Vector(-5, 5), Vector(15, 5)));
  }

  // A line fully outside the rect does not intersect.
  {
    ST_ASSERT("segment outside rect does not intersect",
              !collision::intersects_line(rect, Vector(-5, -5), Vector(-1, -1)));
  }

  // A segment crossing the right edge (between top and bottom) intersects.
  {
    ST_ASSERT("segment crossing right edge intersects",
              collision::intersects_line(rect, Vector(5, 5), Vector(20, 5)));
  }

  // A segment completely inside the rect does NOT cross any edge, so
  // intersects_line (which tests boundary crossings, not containment) is
  // false. This documents the function's semantics.
  {
    ST_ASSERT("segment fully inside rect crosses no edge",
              !collision::intersects_line(rect, Vector(2, 2), Vector(8, 8)));
  }

  // A segment whose endpoint lies exactly on a corner touches the boundary.
  {
    ST_ASSERT("segment ending on corner intersects",
              collision::intersects_line(rect, Vector(10, 10), Vector(15, 15)));
  }

  // --- rectangle_aatriangle -------------------------------------------------

  // Unit square rect at (0,0)-(10,10); a SOUTHWEST AATriangle fills the lower
  // left corner (hypotenuse from top-left to bottom-right). A rect overlapping
  // that corner must collide.
  {
    AATriangle tri(Rectf(0.0f, 0.0f, 10.0f, 10.0f), AATriangle::SOUTHWEST);
    Rectf r(0.0f, 8.0f, 2.0f, 10.0f); // bottom-left sliver, inside the triangle
    collision::Constraints c;
    bool hit = collision::rectangle_aatriangle(&c, r, tri);
    ST_ASSERT("rect inside SW triangle collides", hit);
  }

  // A rect far from the triangle (top-right corner, outside the SW triangle)
  // must not collide.
  {
    AATriangle tri(Rectf(0.0f, 0.0f, 10.0f, 10.0f), AATriangle::SOUTHWEST);
    Rectf r(8.0f, 0.0f, 10.0f, 2.0f); // top-right sliver, outside the triangle
    collision::Constraints c;
    bool hit = collision::rectangle_aatriangle(&c, r, tri);
    ST_ASSERT("rect outside SW triangle does not collide", !hit);
  }
}

/* EOF */
