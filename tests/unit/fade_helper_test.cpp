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

// Coverage for util/fade_helper.cpp: timed value interpolation with an
// optional bound output variable. Only depends on math/easing.

#include <gtest/gtest.h>

#include "util/fade_helper.hpp"

namespace {

constexpr float EPS = 1e-4f;

} // namespace

TEST(FadeHelperTest, starts_at_start_value_not_completed)
{
  FadeHelper fade(1.0f, 1.0f, 0.0f);
  EXPECT_FLOAT_EQ(0.0f, fade.get_value());
  EXPECT_FALSE(fade.completed());
}

TEST(FadeHelperTest, linear_interpolation_midpoint)
{
  FadeHelper fade(1.0f, 1.0f, 0.0f, LinearInterpolation);
  // Halfway through: value should be ~0.5.
  float const v = fade.update(0.5f);
  EXPECT_NEAR(0.5f, v, EPS);
  EXPECT_NEAR(0.5f, fade.get_value(), EPS);
  EXPECT_FALSE(fade.completed());
}

TEST(FadeHelperTest, completes_at_target_after_total_time)
{
  float value = 0.0f;
  FadeHelper fade(&value, 1.0f, 2.0f);

  fade.update(0.25f);
  fade.update(0.25f);
  EXPECT_FALSE(fade.completed());

  // Overshooting dt clamps to target and marks complete.
  float const v = fade.update(10.0f);
  EXPECT_NEAR(2.0f, v, EPS);
  EXPECT_NEAR(2.0f, value, EPS);
  EXPECT_TRUE(fade.completed());
}

TEST(FadeHelperTest, bound_value_tracks_progress)
{
  float value = 3.0f;
  FadeHelper fade(&value, 2.0f, 5.0f, LinearInterpolation);

  fade.update(1.0f);
  EXPECT_NEAR(4.0f, value, EPS);
  fade.update(1.0f);
  EXPECT_NEAR(5.0f, value, EPS);
}

TEST(FadeHelperTest, update_after_completion_stays_at_target)
{
  FadeHelper fade(1.0f, 7.0f, 0.0f);
  fade.update(2.0f);
  ASSERT_TRUE(fade.completed());

  // Repeated updates must not drift past the target.
  fade.update(1.0f);
  fade.update(1.0f);
  EXPECT_NEAR(7.0f, fade.get_value(), EPS);
}

TEST(FadeHelperTest, unbound_helper_returns_value_without_side_effects)
{
  FadeHelper fade(1.0f, 9.0f);
  float const v = fade.update(1.0f);
  EXPECT_NEAR(9.0f, v, EPS); // completed() clamps to target
  EXPECT_NEAR(9.0f, fade.get_value(), EPS);
}

TEST(FadeHelperTest, ease_function_is_applied)
{
  // With QuadraticEaseOut (sqrt-like), halfway in time the eased value
  // is FURTHER along than linear 0.5.
  FadeHelper linear(1.0f, 1.0f, 0.0f, LinearInterpolation);
  FadeHelper eased(1.0f, 1.0f, 0.0f, QuadraticEaseOut);
  linear.update(0.5f);
  eased.update(0.5f);
  EXPECT_GT(eased.get_value(), linear.get_value());
}

TEST(FadeHelperTest, negative_delta_time_before_completion_is_harmless)
{
  FadeHelper fade(1.0f, 1.0f, 0.0f, LinearInterpolation);
  fade.update(-0.5f); // time goes backwards; must not crash or complete
  EXPECT_FALSE(fade.completed());
  EXPECT_NEAR(-0.0f, fade.get_value(), 0.51f); // clamped interpolation of t<0 stays near start
}

TEST(FadeHelperTest, copy_constructor_copies_state)
{
  float value = 0.0f;
  FadeHelper original(&value, 2.0f, 4.0f, LinearInterpolation);
  original.update(0.5f); // progress 1.0

  FadeHelper copy(&original);
  EXPECT_NEAR(original.get_value(), copy.get_value(), EPS);
  EXPECT_FALSE(copy.completed());
}

/* EOF */
