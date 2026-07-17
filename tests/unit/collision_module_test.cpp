//  SuperTux
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

// Regression coverage for the header-only collision::Constraints class and the
// geometry helpers in src/collision/collision.cpp
// (set_rectangle_rectangle_constraints, line_intersects_line, intersects_line,
// rectangle_aatriangle). Previously NONE of this code was exercised by the
// fork test suite (collision_test.cpp only tested math/rectf overlaps).

#include <limits>
#include <cmath>

#include <gtest/gtest.h>

#include "collision/collision.hpp"
#include "math/aatriangle.hpp"
#include "math/rectf.hpp"

namespace {

constexpr float INF = std::numeric_limits<float>::infinity();

// ---------------------------------------------------------------------------
// collision::Constraints
// ---------------------------------------------------------------------------

TEST(CollisionConstraintsTest, default_has_no_constraints)
{
  collision::Constraints c;
  EXPECT_FALSE(c.has_constraints());
  EXPECT_FLOAT_EQ(c.get_position_left(), -INF);
  EXPECT_FLOAT_EQ(c.get_position_right(), INF);
  EXPECT_FLOAT_EQ(c.get_position_top(), -INF);
  EXPECT_FLOAT_EQ(c.get_position_bottom(), INF);
  EXPECT_FLOAT_EQ(c.get_height(), INF - (-INF));
  EXPECT_FLOAT_EQ(c.get_width(), INF - (-INF));
  // With both horizontal bounds at infinity the midpoint is undefined (inf-inf).
  EXPECT_TRUE(std::isnan(c.get_x_midpoint()));
}

TEST(CollisionConstraintsTest, constrain_left_keeps_max)
{
  collision::Constraints c;
  c.constrain_left(3.0f);
  c.constrain_left(5.0f);
  EXPECT_TRUE(c.has_constraints());
  EXPECT_FLOAT_EQ(c.get_position_left(), 5.0f);
  // right/top/bottom untouched
  EXPECT_FLOAT_EQ(c.get_position_right(), INF);
}

TEST(CollisionConstraintsTest, constrain_left_accepts_negative)
{
  collision::Constraints c;
  c.constrain_left(-3.0f); // max(-inf, -3) == -3
  EXPECT_TRUE(c.has_constraints());
  EXPECT_FLOAT_EQ(c.get_position_left(), -3.0f);
}

TEST(CollisionConstraintsTest, constrain_right_keeps_min)
{
  collision::Constraints c;
  c.constrain_right(3.0f);
  c.constrain_right(1.0f);
  EXPECT_TRUE(c.has_constraints());
  EXPECT_FLOAT_EQ(c.get_position_right(), 1.0f);
}

TEST(CollisionConstraintsTest, size_accessors)
{
  collision::Constraints c;
  c.constrain_left(2.0f);
  c.constrain_right(10.0f);
  c.constrain_top(1.0f);
  c.constrain_bottom(9.0f);
  EXPECT_FLOAT_EQ(c.get_width(), 8.0f);
  EXPECT_FLOAT_EQ(c.get_height(), 8.0f);
  EXPECT_FLOAT_EQ(c.get_x_midpoint(), 6.0f);
}

TEST(CollisionConstraintsTest, merge_constraints_takes_extreme_and_or_hits)
{
  collision::Constraints a;
  a.constrain_left(2.0f);
  a.constrain_top(1.0f);
  a.hit.left = true;
  a.hit.crush = true;

  collision::Constraints b;
  b.constrain_right(8.0f);
  b.constrain_bottom(7.0f);
  b.hit.right = true;
  b.hit.bottom = true;
  b.hit.crush = false; // should remain true after OR-merge

  a.merge_constraints(b);

  EXPECT_FLOAT_EQ(a.get_position_left(), 2.0f);   // unchanged (a was tighter)
  EXPECT_FLOAT_EQ(a.get_position_right(), 8.0f);  // taken from b
  EXPECT_FLOAT_EQ(a.get_position_top(), 1.0f);
  EXPECT_FLOAT_EQ(a.get_position_bottom(), 7.0f);
  EXPECT_TRUE(a.hit.left);
  EXPECT_TRUE(a.hit.right);
  EXPECT_TRUE(a.hit.bottom);
  EXPECT_TRUE(a.hit.crush); // OR of true|false == true
}

// ---------------------------------------------------------------------------
// CollisionHit (used as the hit member of Constraints)
// ---------------------------------------------------------------------------

TEST(CollisionHitTest, direction_flags)
{
  CollisionHit h;
  EXPECT_FALSE(h.has_direction());
  EXPECT_FALSE(h.is_vertical());
  EXPECT_FALSE(h.is_horizontal());

  h.bottom = true;
  EXPECT_TRUE(h.has_direction());
  EXPECT_TRUE(h.is_vertical());
  EXPECT_FALSE(h.is_horizontal());

  h = CollisionHit{};
  h.left = true;
  EXPECT_TRUE(h.has_direction());
  EXPECT_FALSE(h.is_vertical());
  EXPECT_TRUE(h.is_horizontal());
}

// ---------------------------------------------------------------------------
// set_rectangle_rectangle_constraints
// ---------------------------------------------------------------------------

TEST(SetRectangleRectangleConstraintsTest, overlapping_picks_horizontal)
{
  // Equal vertical & horizontal penetration -> horizontal branch,
  // rect moving right stops at r2's left edge.
  Rectf r1(0.0f, 0.0f, 10.0f, 10.0f);
  Rectf r2(8.0f, 8.0f, 20.0f, 20.0f);
  collision::Constraints c;
  set_rectangle_rectangle_constraints(&c, r1, r2);
  EXPECT_TRUE(c.hit.right);
  EXPECT_FLOAT_EQ(c.get_position_right(), 8.0f);
}

TEST(SetRectangleRectangleConstraintsTest, overlap_from_above_stops_at_top)
{
  // r2 sits above r1; vertical penetration is the smaller one.
  Rectf r1(0.0f, 0.0f, 10.0f, 10.0f);
  Rectf r2(2.0f, -5.0f, 20.0f, 5.0f);
  collision::Constraints c;
  set_rectangle_rectangle_constraints(&c, r1, r2);
  EXPECT_TRUE(c.hit.top);
  EXPECT_FLOAT_EQ(c.get_position_top(), 5.0f);
}

TEST(SetRectangleRectangleConstraintsTest, overlap_from_below_stops_at_bottom)
{
  // r2 sits below r1; vertical penetration is the smaller one.
  Rectf r1(0.0f, 0.0f, 10.0f, 10.0f);
  Rectf r2(2.0f, 12.0f, 20.0f, 20.0f);
  collision::Constraints c;
  set_rectangle_rectangle_constraints(&c, r1, r2);
  EXPECT_TRUE(c.hit.bottom);
  EXPECT_FLOAT_EQ(c.get_position_bottom(), 12.0f);
}

// ---------------------------------------------------------------------------
// line_intersects_line / intersects_line
// ---------------------------------------------------------------------------

TEST(LineIntersectsLineTest, crossing_diagonals_intersect)
{
  // (0,0)-(10,10) crosses (0,10)-(10,0)
  EXPECT_TRUE(collision::line_intersects_line(
    {0.0f, 0.0f}, {10.0f, 10.0f},
    {0.0f, 10.0f}, {10.0f, 0.0f}));
}

TEST(LineIntersectsLineTest, parallel_distinct_do_not_intersect)
{
  EXPECT_FALSE(collision::line_intersects_line(
    {0.0f, 0.0f}, {10.0f, 0.0f},
    {0.0f, 5.0f}, {10.0f, 5.0f}));
}

TEST(LineIntersectsLineTest, collinear_overlapping_intersect)
{
  EXPECT_TRUE(collision::line_intersects_line(
    {0.0f, 0.0f}, {10.0f, 0.0f},
    {5.0f, 0.0f}, {15.0f, 0.0f}));
}

TEST(IntersectsLineTest, vertical_line_through_rect)
{
  Rectf r(0.0f, 0.0f, 10.0f, 10.0f);
  EXPECT_TRUE(collision::intersects_line(r, {5.0f, -5.0f}, {5.0f, 15.0f}));
}

TEST(IntersectsLineTest, line_missing_rect)
{
  Rectf r(0.0f, 0.0f, 10.0f, 10.0f);
  EXPECT_FALSE(collision::intersects_line(r, {20.0f, 20.0f}, {25.0f, 25.0f}));
}

// ---------------------------------------------------------------------------
// rectangle_aatriangle (object == nullptr path; the slope collision math)
// ---------------------------------------------------------------------------

TEST(RectangleAATriangleTest, no_overlap_returns_false)
{
  // rect far away from triangle bbox -> early-out, no hit set
  Rectf rect(0.0f, 0.0f, 10.0f, 10.0f);
  AATriangle tri(Rectf(100.0f, 100.0f, 110.0f, 110.0f), AATriangle::SOUTHWEST);
  collision::Constraints c;
  bool hits_rectangle_bottom = false;
  EXPECT_FALSE(collision::rectangle_aatriangle(&c, rect, tri, hits_rectangle_bottom));
  EXPECT_FALSE(c.hit.left || c.hit.right || c.hit.top || c.hit.bottom);
}

// Rect at (0,0)-(10,10), triangle bbox (5,5)-(15,15). The rect overlaps the
// solid half of each triangle; the resulting constraint side is the outward
// normal direction. These are regression-locked against the current (correct)
// slope-collision behaviour.
TEST(RectangleAATriangleTest, southwest_constrains_right)
{
  Rectf rect(0.0f, 0.0f, 10.0f, 10.0f);
  AATriangle tri(Rectf(5.0f, 5.0f, 15.0f, 15.0f), AATriangle::SOUTHWEST);
  collision::Constraints c;
  bool hits = false;
  EXPECT_TRUE(collision::rectangle_aatriangle(&c, rect, tri, hits));
  EXPECT_TRUE(c.hit.right);
  EXPECT_FALSE(c.hit.left);
  EXPECT_FLOAT_EQ(c.get_position_right(), 5.0f);
}

TEST(RectangleAATriangleTest, northeast_constrains_right)
{
  Rectf rect(0.0f, 0.0f, 10.0f, 10.0f);
  AATriangle tri(Rectf(5.0f, 5.0f, 15.0f, 15.0f), AATriangle::NORTHEAST);
  collision::Constraints c;
  bool hits = false;
  EXPECT_TRUE(collision::rectangle_aatriangle(&c, rect, tri, hits));
  EXPECT_TRUE(c.hit.right);
  EXPECT_FALSE(c.hit.left);
}

TEST(RectangleAATriangleTest, southeast_triggers_bottom_corner_hack)
{
  Rectf rect(0.0f, 0.0f, 10.0f, 10.0f);
  AATriangle tri(Rectf(5.0f, 5.0f, 15.0f, 15.0f), AATriangle::SOUTHEAST);
  collision::Constraints c;
  bool hits = false;
  EXPECT_TRUE(collision::rectangle_aatriangle(&c, rect, tri, hits));
  // The rect corner lands inside the RDELTA band -> bottom constraint via
  // the corner-fallthrough hack.
  EXPECT_TRUE(hits);
  EXPECT_TRUE(c.hit.bottom);
}

TEST(RectangleAATriangleTest, northwest_constrains_right)
{
  Rectf rect(0.0f, 0.0f, 10.0f, 10.0f);
  AATriangle tri(Rectf(5.0f, 5.0f, 15.0f, 15.0f), AATriangle::NORTHWEST);
  collision::Constraints c;
  bool hits = false;
  EXPECT_TRUE(collision::rectangle_aatriangle(&c, rect, tri, hits));
  EXPECT_TRUE(c.hit.right);
  EXPECT_FALSE(c.hit.left);
}

} // namespace

/* EOF */
