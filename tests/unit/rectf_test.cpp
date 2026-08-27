//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

// Behavior coverage for math/rectf.cpp (the Rectf geometry class). Covers the
// full public API: construction, contains/overlaps, move/moved, the
// set_left/right/top/bottom edge setters, grow/shrink (grown), distance to
// point/rect, empty(), operator==, get_middle, set_pos, set_p1/p2, from_center
// and the SDL_FRect <-> Rectf conversions. Links rectf.cpp + rect.cpp plus the
// SDL3 headers (Rectf carries an SDL_FRect ctor and to_sdl()).

#include <gtest/gtest.h>

#include "math/rectf.hpp"
#include "math/rect.hpp"

TEST(RectfTest, contains_point)
{
  ASSERT_TRUE(Rectf(100.0f, 100.0f, 200.0f, 200.0f).contains(Vector(150.0f, 150.0f)));
  // On the exclusive right/bottom edge is NOT contained.
  ASSERT_FALSE(Rectf(100.0f, 100.0f, 200.0f, 200.0f).contains(Vector(200.0f, 150.0f)));
  ASSERT_FALSE(Rectf(100.0f, 100.0f, 200.0f, 200.0f).contains(Vector(250.0f, 150.0f)));
}

TEST(RectfTest, overlaps_rect)
{
  ASSERT_TRUE(Rectf(100.0f, 100.0f, 200.0f, 200.0f).overlaps(Rectf(150.0f, 150.0f, 190.0f, 190.0f)));
  ASSERT_TRUE(Rectf(100.0f, 100.0f, 200.0f, 200.0f).overlaps(Rectf(100.0f, 100.0f, 200.0f, 200.0f)));
  // SuperTux overlaps() is inclusive on the edges (touching counts as overlap).
  ASSERT_TRUE(Rectf(100.0f, 100.0f, 200.0f, 200.0f).overlaps(Rectf(200.0f, 100.0f, 300.0f, 200.0f)));
  ASSERT_FALSE(Rectf(100.0f, 100.0f, 200.0f, 200.0f).overlaps(Rectf(250.0f, 250.0f, 300.0f, 300.0f)));
}

TEST(RectfTest, empty)
{
  // Zero or negative width/height is empty.
  EXPECT_TRUE(Rectf(0.0f, 0.0f, 0.0f, 10.0f).empty());
  EXPECT_TRUE(Rectf(0.0f, 0.0f, 10.0f, 0.0f).empty());
  EXPECT_TRUE(Rectf(10.0f, 10.0f, 0.0f, 0.0f).empty());
  EXPECT_FALSE(Rectf(0.0f, 0.0f, 10.0f, 10.0f).empty());
}

TEST(RectfTest, moved)
{
  ASSERT_EQ(Rectf(0.0f, 0.0f, 100.0f, 300.0f).moved(Vector(16.0f, 32.0f)), Rectf(16.0f, 32.0f, 116.0f, 332.0f));
  ASSERT_EQ(Rectf(0.0f, 0.0f, 100.0f, 300.0f).moved(Vector(-16.0f, -32.0f)), Rectf(-16.0f, -32.0f, 84.0f, 268.0f));
}

TEST(RectfTest, move_mutates)
{
  Rectf r(0.0f, 0.0f, 10.0f, 10.0f);
  r.move(Vector(5.0f, -3.0f));
  ASSERT_EQ(r, Rectf(5.0f, -3.0f, 15.0f, 7.0f));
}

TEST(RectfTest, set_leftrighttopbottom)
{
  Rectf rect(0.0f, 50.0f, 100.0f, 150.0f);

  rect.set_left(10.0f);
  ASSERT_EQ(Rectf(10.0f, 50.0f, 100.0f, 150.0f), rect);

  rect.set_right(200.0f);
  ASSERT_EQ(Rectf(10.0f, 50.0f, 200.0f, 150.0f), rect);

  rect.set_top(100.0f);
  ASSERT_EQ(Rectf(10.0f, 100.0f, 200.0f, 150.0f), rect);

  rect.set_bottom(200.0f);
  ASSERT_EQ(Rectf(10.0f, 100.0f, 200.0f, 200.0f), rect);
}

TEST(RectfTest, size)
{
  ASSERT_EQ(Rectf(50.0f, 50.0f, 100.0f, 300.0f).get_width(), 50.0f);
  ASSERT_EQ(Rectf(50.0f, 50.0f, 100.0f, 300.0f).get_height(), 250.0f);
}

TEST(RectfTest, from_center)
{
  ASSERT_EQ(Rectf::from_center({16.0f, 16.0f}, {32.0f, 32.0f}), Rectf(0.0f, 0.0f, 32.0f, 32.0f));
}

TEST(RectfTest, set_p1)
{
  Rectf rect(Vector(16.0f, 16.0f), Vector(32.0f, 32.0f));
  rect.set_p1({1.0f, 5.0f});
  ASSERT_EQ(Rectf(Vector(1.0f, 5.0f), Vector(32.0f, 32.0f)), rect);
}

TEST(RectfTest, set_p2)
{
  Rectf rect(Vector(16.0f, 16.0f), Vector(32.0f, 32.0f));
  rect.set_p2({48.0f, 100.0f});
  ASSERT_EQ(Rectf(Vector(16.0f, 16.0f), Vector(48.0f, 100.0f)), rect);
}

TEST(RectfTest, equality)
{
  EXPECT_TRUE(Rectf(1.0f, 2.0f, 3.0f, 4.0f) == Rectf(1.0f, 2.0f, 3.0f, 4.0f));
  // Same p1 and same p2 (= same size) compares equal.
  EXPECT_TRUE(Rectf(1.0f, 2.0f, 3.0f, 4.0f) == Rectf(1.0f, 2.0f, 3.0f, 4.0f));
  // Different p2 (different size) makes them unequal.
  EXPECT_FALSE(Rectf(1.0f, 2.0f, 3.0f, 4.0f) == Rectf(1.0f, 2.0f, 3.0f, 5.0f));
}

TEST(RectfTest, grow_uniform)
{
  // grow(b) expands by b on every side.
  const Rectf r = Rectf(10.0f, 10.0f, 20.0f, 20.0f).grown(5.0f);
  EXPECT_EQ(r, Rectf(5.0f, 5.0f, 25.0f, 25.0f));
}

TEST(RectfTest, grow_vector)
{
  const Rectf r = Rectf(10.0f, 10.0f, 20.0f, 20.0f).grown(Vector(2.0f, 4.0f));
  EXPECT_EQ(r, Rectf(8.0f, 6.0f, 22.0f, 24.0f));
}

TEST(RectfTest, grow_negative_shrinks)
{
  // Shrinking by an amount that keeps size >= 0 actually shrinks both sides.
  const Rectf r = Rectf(10.0f, 10.0f, 20.0f, 20.0f).grown(-3.0f);
  EXPECT_EQ(r, Rectf(13.0f, 13.0f, 17.0f, 17.0f));
}

TEST(RectfTest, grow_negative_below_zero_is_noop)
{
  // Shrinking past zero is a no-op (returns the original rect unchanged).
  const Rectf base(10.0f, 10.0f, 20.0f, 20.0f);
  EXPECT_EQ(base.grown(-8.0f), base);
}

TEST(RectfTest, get_middle_and_set_pos)
{
  Rectf r(0.0f, 0.0f, 10.0f, 20.0f);
  EXPECT_EQ(r.get_middle(), Vector(5.0f, 10.0f));
  r.set_pos(Vector(100.0f, 200.0f));
  EXPECT_EQ(r, Rectf(100.0f, 200.0f, 110.0f, 220.0f));
}

TEST(RectfTest, sdl_roundtrip)
{
  const Rectf r(1.0f, 2.0f, 3.0f, 4.0f);
  const SDL_FRect sdl = r.to_sdl();
  EXPECT_FLOAT_EQ(sdl.x, 1.0f);
  EXPECT_FLOAT_EQ(sdl.y, 2.0f);
  EXPECT_FLOAT_EQ(sdl.w, 2.0f);
  EXPECT_FLOAT_EQ(sdl.h, 2.0f);
  // Back-conversion must reproduce the original rect.
  const Rectf back(sdl);
  EXPECT_EQ(back, r);
}

// vim: set ts=2 sw=2 et :

TEST(RectfTest, distance_to_anchor_point)
{
  // distance(Vector) measures the distance from the rect's anchor point
  // (default = ANCHOR_MIDDLE) to the given point.
  Rectf r(0.0f, 0.0f, 10.0f, 10.0f);
  // Default anchor is the middle of the rect (5, 5).
  EXPECT_NEAR(r.distance(Vector(5.0f, 12.0f)), 7.0f, 1e-4f);
  EXPECT_NEAR(r.distance(Vector(12.0f, 5.0f)), 7.0f, 1e-4f);
  EXPECT_NEAR(r.distance(Vector(5.0f, 5.0f)), 0.0f, 1e-4f);
}

TEST(RectfTest, distance_to_anchor_point_explicit)
{
  // Explicit anchor points: ANCHOR_TOP_LEFT = (0,0), ANCHOR_BOTTOM_RIGHT = (10,10).
  Rectf r(0.0f, 0.0f, 10.0f, 10.0f);
  EXPECT_NEAR(r.distance(Vector(3.0f, 3.0f), ANCHOR_TOP_LEFT), glm::distance(Vector(0.0f, 0.0f), Vector(3.0f, 3.0f)), 1e-4f);
  EXPECT_NEAR(r.distance(Vector(7.0f, 7.0f), ANCHOR_BOTTOM_RIGHT), glm::distance(Vector(10.0f, 10.0f), Vector(7.0f, 7.0f)), 1e-4f);
  EXPECT_NEAR(r.distance(Vector(0.0f, 0.0f), ANCHOR_TOP_LEFT), 0.0f, 1e-4f);
}

TEST(RectfTest, distance_rect_to_rect_default_anchor)
{
  // distance(Rectf) uses the default anchor (middle) of each rect.
  Rectf a(0.0f, 0.0f, 10.0f, 10.0f);   // middle (5,5)
  Rectf b(20.0f, 0.0f, 30.0f, 10.0f);  // middle (25,5)
  // Distance between middles: sqrt((25-5)^2 + (5-5)^2) = 20.
  EXPECT_NEAR(a.distance(b), 20.0f, 1e-4f);
}

// --- .cpp implementation coverage (rectf.cpp) ---------------------------
// These exercise the three functions defined in src/math/rectf.cpp, which
// the header-only API tests above do not reach.

TEST(RectfTest, ctor_from_int_rect)
{
  // Rectf(const Rect&) converts integer edges to floats.
  Rect ri(10, 20, 110, 220);  // left, top, right, bottom
  Rectf rf(ri);
  EXPECT_FLOAT_EQ(rf.get_left(), 10.0f);
  EXPECT_FLOAT_EQ(rf.get_top(), 20.0f);
  EXPECT_FLOAT_EQ(rf.get_right(), 110.0f);
  EXPECT_FLOAT_EQ(rf.get_bottom(), 220.0f);
  EXPECT_FLOAT_EQ(rf.get_width(), 100.0f);
  EXPECT_FLOAT_EQ(rf.get_height(), 200.0f);
}

TEST(RectfTest, to_rect_rounds_down)
{
  // to_rect() truncates float edges to ints.
  Rectf rf(10.5f, 20.9f, 110.4f, 220.1f);
  Rect ri = rf.to_rect();
  EXPECT_EQ(ri.left, 10);
  EXPECT_EQ(ri.top, 20);
  EXPECT_EQ(ri.right, 110);
  EXPECT_EQ(ri.bottom, 220);
}

TEST(RectfTest, stream_operator)
{
  // operator<< prints "Rectf(l, t, r, b)".
  Rectf rf(1.0f, 2.0f, 3.0f, 4.0f);
  std::ostringstream os;
  os << rf;
  EXPECT_EQ(os.str(), "Rectf(1, 2, 3, 4)");
}
