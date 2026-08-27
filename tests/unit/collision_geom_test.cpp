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
// intersects_line (Rect vs segment), and rectangle_aatriangle. Converted
// from ST_ASSERT harness to GoogleTest for better failure diagnostics.

#include <gtest/gtest.h>

#include "collision/collision.hpp"
#include "math/aatriangle.hpp"
#include "math/rectf.hpp"

#include <cmath>

namespace {

bool approx(float a, float b, float eps = 0.001f)
{
  return std::fabs(a - b) < eps;
}

} // namespace

// --- line_intersects_line --------------------------------------------------

TEST(CollisionGeomTest, crossing_lines_intersect)
{
  EXPECT_TRUE(collision::line_intersects_line(
    Vector(-1, 0), Vector(1, 0),
    Vector(0, -1), Vector(0, 1)));
}

TEST(CollisionGeomTest, parallel_lines_do_not_intersect)
{
  EXPECT_FALSE(collision::line_intersects_line(
    Vector(-1, 0), Vector(1, 0),
    Vector(-1, 1), Vector(1, 1)));
}

TEST(CollisionGeomTest, collinear_overlapping_lines_intersect)
{
  EXPECT_TRUE(collision::line_intersects_line(
    Vector(-2, 0), Vector(0, 0),
    Vector(0, 0), Vector(2, 0)));
}

TEST(CollisionGeomTest, endpoint_touching_lines_intersect)
{
  EXPECT_TRUE(collision::line_intersects_line(
    Vector(0, 0), Vector(2, 0),
    Vector(0, 0), Vector(0, 2)));
}

TEST(CollisionGeomTest, non_crossing_non_parallel_lines_do_not_intersect)
{
  EXPECT_FALSE(collision::line_intersects_line(
    Vector(0, 0), Vector(1, 0),
    Vector(2, 2), Vector(3, 3)));
}

// --- intersects_line (Rect vs segment) ------------------------------------

TEST(CollisionGeomTest, segment_through_rect_intersects)
{
  Rectf rect(0.0f, 0.0f, 10.0f, 10.0f);
  EXPECT_TRUE(collision::intersects_line(rect, Vector(-5, 5), Vector(15, 5)));
}

TEST(CollisionGeomTest, segment_outside_rect_does_not_intersect)
{
  Rectf rect(0.0f, 0.0f, 10.0f, 10.0f);
  EXPECT_FALSE(collision::intersects_line(rect, Vector(-5, -5), Vector(-1, -1)));
}

TEST(CollisionGeomTest, segment_crossing_right_edge_intersects)
{
  Rectf rect(0.0f, 0.0f, 10.0f, 10.0f);
  EXPECT_TRUE(collision::intersects_line(rect, Vector(5, 5), Vector(20, 5)));
}

TEST(CollisionGeomTest, segment_fully_inside_rect_crosses_no_edge)
{
  Rectf rect(0.0f, 0.0f, 10.0f, 10.0f);
  EXPECT_FALSE(collision::intersects_line(rect, Vector(2, 2), Vector(8, 8)));
}

TEST(CollisionGeomTest, segment_ending_on_corner_intersects)
{
  Rectf rect(0.0f, 0.0f, 10.0f, 10.0f);
  EXPECT_TRUE(collision::intersects_line(rect, Vector(10, 10), Vector(15, 15)));
}

// --- rectangle_aatriangle -------------------------------------------------

TEST(CollisionGeomTest, rect_inside_SW_triangle_collides)
{
  AATriangle tri(Rectf(0.0f, 0.0f, 10.0f, 10.0f), AATriangle::SOUTHWEST);
  Rectf r(0.0f, 8.0f, 2.0f, 10.0f);
  collision::Constraints c;
  EXPECT_TRUE(collision::rectangle_aatriangle(&c, r, tri));
}

TEST(CollisionGeomTest, rect_outside_SW_triangle_does_not_collide)
{
  AATriangle tri(Rectf(0.0f, 0.0f, 10.0f, 10.0f), AATriangle::SOUTHWEST);
  Rectf r(8.0f, 0.0f, 10.0f, 2.0f);
  collision::Constraints c;
  EXPECT_FALSE(collision::rectangle_aatriangle(&c, r, tri));
}