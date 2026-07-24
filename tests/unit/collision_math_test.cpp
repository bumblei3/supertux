//  SuperTux
//  Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
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

// Pure-logic coverage for the freestanding collision math in
// collision/collision.cpp. These routines are the core of SuperTux's
// per-frame collision resolution (AABB vs AABB pushing, axis-aligned-triangle
// slope collision, and line/segment intersection used for line-of-sight and
// beam checks) yet had zero unit tests. They only depend on Rectf/AATriangle/
// Vector, so they link without the Sector/Player/Editor engine stack.

#include "collision/collision.hpp"
#include "math/aatriangle.hpp"
#include "math/rectf.hpp"

#include <gtest/gtest.h>

using namespace collision;

// ---------------------------------------------------------------------------
// set_rectangle_rectangle_constraints: AABB-vs-AABB resolution
// ---------------------------------------------------------------------------

// When the vertical penetration is smaller than the horizontal one, the moving
// rect must be pushed out vertically (nearest face wins).
TEST(CollisionMathTest, rectangle_resolution_prefers_smaller_axis)
{
  Constraints c;
  Rectf moving(0.0f, 0.0f, 10.0f, 10.0f);   // 10x10 box at origin
  Rectf solid(5.0f, 6.0f, 20.0f, 20.0f);    // overlaps moving bottom-right
  // vertical penetration   = min(10-6, 20-0) = 4
  // horizontal penetration = min(10-5, 20-0) = 5  -> vertical wins
  set_rectangle_rectangle_constraints(&c, moving, solid);

  EXPECT_TRUE(c.has_constraints());
  // itop(4) < ibottom(20) => constrain_bottom(solid.top = 6)
  EXPECT_FLOAT_EQ(c.get_position_bottom(), 6.0f);
  EXPECT_TRUE(c.hit.bottom);
  EXPECT_FALSE(c.hit.top);
}

TEST(CollisionMathTest, rectangle_resolution_horizontal_axis)
{
  Constraints c;
  Rectf moving(0.0f, 0.0f, 10.0f, 10.0f);
  Rectf solid(6.0f, 5.0f, 20.0f, 9.0f);
  // vertical penetration   = min(10-5, 9-0) = 5
  // horizontal penetration = min(10-6, 20-0) = 4  -> horizontal wins
  set_rectangle_rectangle_constraints(&c, moving, solid);

  // ileft(4) < iright(20) => constrain_right(solid.left = 6)
  EXPECT_FLOAT_EQ(c.get_position_right(), 6.0f);
  EXPECT_TRUE(c.hit.right);
  EXPECT_FALSE(c.hit.left);
}

// The function has no early-out: it always writes a constraint (the engine
// only calls it for already-overlapping pairs). A rect resting exactly on top
// of a solid tile (touching at y=10) must be pushed down onto the tile top.
TEST(CollisionMathTest, rectangle_resolution_resting_on_top)
{
  Constraints c;
  Rectf moving(0.0f, 0.0f, 10.0f, 10.0f);  // bottom edge at y=10
  Rectf solid(0.0f, 10.0f, 10.0f, 20.0f);  // top edge at y=10
  // vertical penetration   = min(10-10, 20-0) = 0  (smaller)
  // horizontal penetration = min(10-0, 10-0)  = 10
  set_rectangle_rectangle_constraints(&c, moving, solid);

  EXPECT_TRUE(c.has_constraints());
  EXPECT_FLOAT_EQ(c.get_position_bottom(), 10.0f);
  EXPECT_TRUE(c.hit.bottom);
}

// ---------------------------------------------------------------------------
// rectangle_aatriangle: axis-aligned slope collision
// ---------------------------------------------------------------------------

TEST(CollisionMathTest, slope_no_overlap_returns_false)
{
  Constraints c;
  AATriangle tri(Rectf(100.0f, 100.0f, 120.0f, 120.0f), AATriangle::SOUTHWEST);
  Rectf r(0.0f, 0.0f, 10.0f, 10.0f); // far away
  bool hits_bottom = false;
  const bool res = rectangle_aatriangle(&c, r, tri, hits_bottom, nullptr);

  EXPECT_FALSE(res);
  EXPECT_FALSE(c.has_constraints());
}

// A rect resting in the lower corner of a SOUTHWEST (\) slope must register a
// bottom (ground) hit and a left hit, pushing it up-left onto the slope. The
// exact position is a deliberate heuristic (see the "somewhat of a hack"
// comment in collision.cpp), so we assert only the deterministic hit flags.
TEST(CollisionMathTest, slope_southwest_flags_bottom_and_left)
{
  Constraints c;
  AATriangle tri(Rectf(0.0f, 0.0f, 20.0f, 20.0f), AATriangle::SOUTHWEST);
  Rectf r(2.0f, 2.0f, 8.0f, 8.0f);
  bool hits_bottom = false;
  const bool res = rectangle_aatriangle(&c, r, tri, hits_bottom, nullptr);

  EXPECT_TRUE(res);
  EXPECT_TRUE(c.hit.bottom);
  EXPECT_TRUE(hits_bottom);
  EXPECT_TRUE(c.hit.left);
  EXPECT_FALSE(c.hit.right);
  EXPECT_FALSE(c.hit.top);
}

// ---------------------------------------------------------------------------
// line_intersects_line / intersects_line: segment intersection
// ---------------------------------------------------------------------------

TEST(CollisionMathTest, line_intersects_crossing)
{
  const Vector a(0.0f, 0.0f), b(10.0f, 10.0f);
  const Vector c(0.0f, 10.0f), d(10.0f, 0.0f);
  EXPECT_TRUE(line_intersects_line(a, b, c, d));
}

TEST(CollisionMathTest, line_no_intersect_disjoint)
{
  const Vector a(0.0f, 0.0f), b(10.0f, 10.0f);
  const Vector c(100.0f, 100.0f), d(200.0f, 200.0f);
  EXPECT_FALSE(line_intersects_line(a, b, c, d));
}

TEST(CollisionMathTest, line_parallel_no_intersect)
{
  const Vector a(0.0f, 0.0f), b(10.0f, 0.0f);
  const Vector c(0.0f, 5.0f), d(10.0f, 5.0f);
  EXPECT_FALSE(line_intersects_line(a, b, c, d));
}

TEST(CollisionMathTest, rect_intersects_line)
{
  const Rectf r(0.0f, 0.0f, 10.0f, 10.0f);
  // Diagonal through the rect.
  EXPECT_TRUE(intersects_line(r, Vector(-5.0f, -5.0f), Vector(15.0f, 15.0f)));
  // Line well above the rect.
  EXPECT_FALSE(intersects_line(r, Vector(-5.0f, 50.0f), Vector(15.0f, 50.0f)));
}

// ---------------------------------------------------------------------------
// Constraints::merge_constraints: union of bounds + hit flags
// ---------------------------------------------------------------------------

TEST(CollisionMathTest, constraints_merge_union)
{
  Constraints a;
  a.constrain_left(2.0f);
  a.constrain_bottom(3.0f);
  a.hit.left = true;

  Constraints b;
  b.constrain_right(8.0f);
  b.constrain_top(1.0f);
  b.hit.right = true;
  b.hit.crush = true;

  a.merge_constraints(b);

  // left is max(left) => 2, right is min(right) => 8
  EXPECT_FLOAT_EQ(a.get_position_left(), 2.0f);
  EXPECT_FLOAT_EQ(a.get_position_right(), 8.0f);
  EXPECT_FLOAT_EQ(a.get_position_top(), 1.0f);
  EXPECT_FLOAT_EQ(a.get_position_bottom(), 3.0f);
  EXPECT_TRUE(a.hit.left);
  EXPECT_TRUE(a.hit.right);
  EXPECT_TRUE(a.hit.crush);
}

// vim: set ts=2 sw=2 et :
