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

#include <gtest/gtest.h>

#include <cmath>
#include <string>

#include "math/easing.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

namespace {

// Helper: assert an easing matches the analytic formula at a set of p values.
void expect_near(double expected, double actual, const char* msg)
{
  // Easing functions are smooth and bounded in [0,1]; a tight tolerance is fine.
  EXPECT_NEAR(expected, actual, 1e-9) << msg;
}

} // namespace

// --- Elementary easing functions (verified against their math definitions) ---

TEST(EasingTest, linear)
{
  expect_near(0.0, LinearInterpolation(0.0), "p=0");
  expect_near(0.5, LinearInterpolation(0.5), "p=0.5");
  expect_near(1.0, LinearInterpolation(1.0), "p=1");
}

TEST(EasingTest, quadratic)
{
  expect_near(0.0, QuadraticEaseIn(0.0), "in p=0");
  expect_near(0.25, QuadraticEaseIn(0.5), "in p=0.5");
  expect_near(1.0, QuadraticEaseIn(1.0), "in p=1");

  expect_near(0.0, QuadraticEaseOut(0.0), "out p=0");
  expect_near(0.75, QuadraticEaseOut(0.5), "out p=0.5");
  expect_near(1.0, QuadraticEaseOut(1.0), "out p=1");

  expect_near(0.0, QuadraticEaseInOut(0.0), "inout p=0");
  expect_near(0.5, QuadraticEaseInOut(0.5), "inout p=0.5");
  expect_near(1.0, QuadraticEaseInOut(1.0), "inout p=1");
  expect_near(0.125, QuadraticEaseInOut(0.25), "inout p=0.25 (2p^2)");
  expect_near(0.875, QuadraticEaseInOut(0.75), "inout p=0.75");
}

TEST(EasingTest, cubic)
{
  expect_near(0.0, CubicEaseIn(0.0), "in p=0");
  expect_near(0.125, CubicEaseIn(0.5), "in p=0.5");
  expect_near(1.0, CubicEaseIn(1.0), "in p=1");

  expect_near(0.0, CubicEaseOut(0.0), "out p=0");
  expect_near(0.875, CubicEaseOut(0.5), "out p=0.5");
  expect_near(1.0, CubicEaseOut(1.0), "out p=1");

  expect_near(0.0, CubicEaseInOut(0.0), "inout p=0");
  expect_near(0.5, CubicEaseInOut(0.5), "inout p=0.5");
  expect_near(1.0, CubicEaseInOut(1.0), "inout p=1");
  expect_near(0.0625, CubicEaseInOut(0.25), "inout p=0.25 (4p^3)");
}

TEST(EasingTest, quartic)
{
  expect_near(0.0, QuarticEaseIn(0.0), "in p=0");
  expect_near(0.0625, QuarticEaseIn(0.5), "in p=0.5");
  expect_near(1.0, QuarticEaseIn(1.0), "in p=1");

  expect_near(0.0, QuarticEaseOut(0.0), "out p=0");
  expect_near(0.9375, QuarticEaseOut(0.5), "out p=0.5");
  expect_near(1.0, QuarticEaseOut(1.0), "out p=1");

  expect_near(0.0, QuarticEaseInOut(0.0), "inout p=0");
  expect_near(0.5, QuarticEaseInOut(0.5), "inout p=0.5");
  expect_near(1.0, QuarticEaseInOut(1.0), "inout p=1");
}

TEST(EasingTest, quintic)
{
  expect_near(0.0, QuinticEaseIn(0.0), "in p=0");
  expect_near(0.03125, QuinticEaseIn(0.5), "in p=0.5");
  expect_near(1.0, QuinticEaseIn(1.0), "in p=1");

  expect_near(0.0, QuinticEaseOut(0.0), "out p=0");
  expect_near(0.96875, QuinticEaseOut(0.5), "out p=0.5");
  expect_near(1.0, QuinticEaseOut(1.0), "out p=1");

  expect_near(0.0, QuinticEaseInOut(0.0), "inout p=0");
  expect_near(0.5, QuinticEaseInOut(0.5), "inout p=0.5");
  expect_near(1.0, QuinticEaseInOut(1.0), "inout p=1");
}

TEST(EasingTest, sine)
{
  expect_near(0.0, SineEaseIn(0.0), "in p=0");
  expect_near(1.0, SineEaseIn(1.0), "in p=1 (sin(-pi/2)+1)");
  expect_near(0.0, SineEaseOut(0.0), "out p=0");
  expect_near(1.0, SineEaseOut(1.0), "out p=1");
  // out(0.5) = sin(pi/4) = sqrt(2)/2.
  expect_near(std::sqrt(2.0) / 2.0, SineEaseOut(0.5), "out p=0.5 (sin pi/4)");
  expect_near(0.0, SineEaseInOut(0.0), "inout p=0");
  expect_near(0.5, SineEaseInOut(0.5), "inout p=0.5 (half wave)");
  expect_near(1.0, SineEaseInOut(1.0), "inout p=1");
}

TEST(EasingTest, circular)
{
  expect_near(0.0, CircularEaseIn(0.0), "in p=0");
  expect_near(1.0, CircularEaseIn(1.0), "in p=1");
  expect_near(1.0 - std::sqrt(1.0 - 0.25), CircularEaseIn(0.5), "in p=0.5");
  expect_near(0.0, CircularEaseOut(0.0), "out p=0");
  expect_near(1.0, CircularEaseOut(1.0), "out p=1");
  expect_near(std::sqrt(0.75), CircularEaseOut(0.5), "out p=0.5");
  expect_near(0.0, CircularEaseInOut(0.0), "inout p=0");
  expect_near(0.5, CircularEaseInOut(0.5), "inout p=0.5");
  expect_near(1.0, CircularEaseInOut(1.0), "inout p=1");
}

TEST(EasingTest, exponential)
{
  expect_near(0.0, ExponentialEaseIn(0.0), "in p=0 (special case)");
  expect_near(1.0, ExponentialEaseIn(1.0), "in p=1");
  expect_near(0.0, ExponentialEaseOut(0.0), "out p=0");
  expect_near(1.0, ExponentialEaseOut(1.0), "out p=1 (special case)");
  // out(0.5) = 1 - 2^(-5) = 31/32.
  expect_near(31.0 / 32.0, ExponentialEaseOut(0.5), "out p=0.5");
  expect_near(0.0, ExponentialEaseInOut(0.0), "inout p=0 (special)");
  expect_near(1.0, ExponentialEaseInOut(1.0), "inout p=1 (special)");
  // inout(0.5): p>=0.5 branch -> -0.5*2^(-10*0.5+10)+1 = -0.5*1+1 = 0.5.
  expect_near(0.5, ExponentialEaseInOut(0.5), "inout p=0.5");
}

TEST(EasingTest, elastic)
{
  expect_near(0.0, ElasticEaseIn(0.0), "in p=0");
  expect_near(1.0, ElasticEaseIn(1.0), "in p=1");
  // out(0) = sin(-13pi/2)*2^0 + 1 = sin(-6.5pi)+1 = sin(-0.5pi)+1 = 0.
  expect_near(0.0, ElasticEaseOut(0.0), "out p=0");
  expect_near(1.0, ElasticEaseOut(1.0), "out p=1");
  // Elastic (like Bounce/Back) deliberately overshoots: the damped-sine
  // formula sin(-13pi/2*(p+1))*2^(-10p)+1 exceeds 1.0 near p=0.5. This is
  // by design (a spring overshoots its target), not a bug: callers apply
  // the result as an animation factor and the value lands exactly on 1.0
  // at p=1.0. Assert the actual overshoot so a future regression that
  // changes the phase/scale stays visible.
  const double eout = ElasticEaseOut(0.5);
  EXPECT_GT(eout, 1.0);
  expect_near(1.0220970869120796, eout, "out p=0.5 (overshoot by design)");
  expect_near(0.0, ElasticEaseInOut(0.0), "inout p=0 (special)");
  expect_near(1.0, ElasticEaseInOut(1.0), "inout p=1 (special)");
  // InOut(0.5) uses a different branch and stays at 0.5.
  expect_near(0.5, ElasticEaseInOut(0.5), "inout p=0.5");
}

TEST(EasingTest, back)
{
  expect_near(0.0, BackEaseIn(0.0), "in p=0");
  expect_near(1.0, BackEaseIn(1.0), "in p=1");
  expect_near(0.0, BackEaseOut(0.0), "out p=0");
  expect_near(1.0, BackEaseOut(1.0), "out p=1");
  // Back overshoots: at p=0.5 In is below linear 0.5, Out above.
  expect_near(0.5 * 0.5 * 0.5 - 0.5 * std::sin(0.5 * M_PI),
              BackEaseIn(0.5), "in p=0.5");
  expect_near(1.0 - (0.5 * 0.5 * 0.5 - 0.5 * std::sin(0.5 * M_PI)),
              BackEaseOut(0.5), "out p=0.5");
  expect_near(0.0, BackEaseInOut(0.0), "inout p=0");
  expect_near(0.5, BackEaseInOut(0.5), "inout p=0.5");
  expect_near(1.0, BackEaseInOut(1.0), "inout p=1");
}

TEST(EasingTest, bounce)
{
  expect_near(0.0, BounceEaseIn(0.0), "in p=0");
  expect_near(1.0, BounceEaseIn(1.0), "in p=1");
  expect_near(0.0, BounceEaseOut(0.0), "out p=0");
  expect_near(1.0, BounceEaseOut(1.0), "out p=1");
  // BounceOut(0.5) lies on the second segment: 363/40*0.25 - 99/10*0.5 + 17/5.
  expect_near((363.0 / 40.0) * 0.25 - (99.0 / 10.0) * 0.5 + 17.0 / 5.0,
              BounceEaseOut(0.5), "out p=0.5");
  expect_near(0.0, BounceEaseInOut(0.0), "inout p=0");
  expect_near(1.0, BounceEaseInOut(1.0), "inout p=1");
  expect_near(0.5, BounceEaseInOut(0.5), "inout p=0.5");
}

// --- Lookup / name round-trips ---

TEST(EasingTest, getEasingByName_matches_direct)
{
  EXPECT_EQ(LinearInterpolation(0.3), getEasingByName(EaseNone)(0.3));
  EXPECT_EQ(QuadraticEaseIn(0.3), getEasingByName(EaseQuadIn)(0.3));
  EXPECT_EQ(BounceEaseOut(0.7), getEasingByName(EaseBounceOut)(0.7));
  // Unknown enum falls back to linear.
  EXPECT_EQ(LinearInterpolation(0.5),
            getEasingByName(static_cast<EasingMode>(-1))(0.5));
}

TEST(EasingTest, name_roundtrip)
{
  for (int i = EaseNone; i <= EaseBounceInOut; ++i)
  {
    EasingMode m = static_cast<EasingMode>(i);
    EXPECT_EQ(m, EasingMode_from_string(getEasingName(m)))
      << "roundtrip failed for mode " << i;
  }
}

TEST(EasingTest, from_string_unknown)
{
  EXPECT_EQ(EaseNone, EasingMode_from_string("NotAnEasing"));
}

TEST(EasingTest, reverse_easing)
{
  // In <-> Out swap.
  EXPECT_EQ(EaseQuadOut, get_reverse_easing(EaseQuadIn));
  EXPECT_EQ(EaseQuadIn, get_reverse_easing(EaseQuadOut));
  // InOut and None are their own reverse.
  EXPECT_EQ(EaseQuadInOut, get_reverse_easing(EaseQuadInOut));
  EXPECT_EQ(EaseNone, get_reverse_easing(EaseNone));
}

/* EOF */
