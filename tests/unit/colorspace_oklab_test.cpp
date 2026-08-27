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

// Coverage for util/colorspace_oklab.hpp (ColorOKLCh): the sRGB -> OKLab LCh
// conversion and the modified-lightness estimate. Reference L values follow
// Björn Ottosson's OKLab definition (https://bottosson.github.io/posts/oklab).
// Logging is stubbed (colorspace_oklab_test_stub.cpp) to stay engine-free.

#include <gtest/gtest.h>

#include "util/colorspace_oklab.hpp"
#include "video/color.hpp"

#include <cmath>

namespace {

float const eps = 0.01f;
float const eps_tight = 0.001f;

bool approx(float a, float b, float tol = eps)
{
  return std::fabs(a - b) < tol;
}

} // namespace

// ── sRGB -> OKLab LCh conversion ─────────────────────────────────────────────

TEST(ColorOKLCh, white_maps_to_L1_and_zero_chroma)
{
  ColorOKLCh w(Color(1.0f, 1.0f, 1.0f));
  EXPECT_NEAR(w.L, 1.0f, eps_tight);
  EXPECT_LT(w.C, 0.01f);
}

TEST(ColorOKLCh, black_maps_to_L0_and_zero_chroma)
{
  ColorOKLCh b(Color(0.0f, 0.0f, 0.0f));
  EXPECT_NEAR(b.L, 0.0f, eps_tight);
  EXPECT_LT(b.C, 0.01f);
  EXPECT_FLOAT_EQ(b.h, 0.0f);
}

TEST(ColorOKLCh, mid_grey_has_zero_chroma_and_forced_hue)
{
  ColorOKLCh g(Color(0.5f, 0.5f, 0.5f));
  EXPECT_LT(g.C, 0.01f);
  EXPECT_FLOAT_EQ(g.h, 0.0f);
  EXPECT_GT(g.L, 0.0f);
  EXPECT_LT(g.L, 1.0f);
}

TEST(ColorOKLCh, pure_red_has_substantial_chroma)
{
  ColorOKLCh r(Color(1.0f, 0.0f, 0.0f));
  EXPECT_NEAR(r.L, 0.628f, 0.02f);
  EXPECT_GT(r.C, 0.2f);
}

TEST(ColorOKLCh, pure_green_has_substantial_chroma)
{
  ColorOKLCh g(Color(0.0f, 1.0f, 0.0f));
  EXPECT_NEAR(g.L, 0.870f, 0.01f);
  EXPECT_GT(g.C, 0.2f);
}

TEST(ColorOKLCh, pure_blue_has_substantial_chroma)
{
  ColorOKLCh b(Color(0.0f, 0.0f, 1.0f));
  EXPECT_NEAR(b.L, 0.452f, 0.01f);
  EXPECT_GT(b.C, 0.2f);
}

TEST(ColorOKLCh, greyscale_ramp_is_monotonic_in_L)
{
  ColorOKLCh dark(Color(0.2f, 0.2f, 0.2f));
  ColorOKLCh light(Color(0.8f, 0.8f, 0.8f));
  EXPECT_LT(dark.L, light.L);
}

TEST(ColorOKLCh, greyscale_colors_have_hue_zero)
{
  for (float v : {0.0f, 0.1f, 0.3f, 0.5f, 0.7f, 0.9f, 1.0f}) {
    ColorOKLCh c(Color(v, v, v));
    EXPECT_FLOAT_EQ(c.h, 0.0f) << "failed at v=" << v;
  }
}

// ── Modified lightness estimate ───────────────────────────────────────────────

TEST(ColorOKLCh, modified_lightness_black_is_zero)
{
  ColorOKLCh black(Color(0.0f, 0.0f, 0.0f));
  EXPECT_NEAR(black.get_modified_lightness(), 0.0f, eps_tight);
}

TEST(ColorOKLCh, modified_lightness_white_is_strictly_greater_than_black)
{
  ColorOKLCh black(Color(0.0f, 0.0f, 0.0f));
  ColorOKLCh white(Color(1.0f, 1.0f, 1.0f));
  EXPECT_GT(white.get_modified_lightness(), black.get_modified_lightness());
}

TEST(ColorOKLCh, modified_lightness_is_monotonic)
{
  ColorOKLCh lows(Color(0.2f, 0.2f, 0.2f));
  ColorOKLCh highs(Color(0.8f, 0.8f, 0.8f));
  EXPECT_LT(lows.get_modified_lightness(), highs.get_modified_lightness());
}

TEST(ColorOKLCh, modified_lightness_increases_with_L)
{
  // Verify monotonic increase of the modified-lightness estimate over the
  // full L range: sample 11 evenly-spaced L values from 0 to 1.
  for (int i = 1; i <= 10; ++i) {
    float L0 = (i - 1) * 0.1f;
    float L1 = i * 0.1f;
    ColorOKLCh c0(L0, 0.0f, 0.0f);
    ColorOKLCh c1(L1, 0.0f, 0.0f);
    EXPECT_LT(c0.get_modified_lightness(), c1.get_modified_lightness())
      << "L0=" << L0 << " L1=" << L1;
  }
}

// ── Explicit (L, C, h) constructor ────────────────────────────────────────────

TEST(ColorOKLCh, explicit_ctor_stores_arguments)
{
  ColorOKLCh c(0.5f, 0.1f, 1.2f);
  EXPECT_NEAR(c.L, 0.5f, eps_tight);
  EXPECT_NEAR(c.C, 0.1f, eps_tight);
  EXPECT_NEAR(c.h, 1.2f, eps_tight);
}

// ── Round-trip consistency ────────────────────────────────────────────────────

TEST(ColorOKLCh, hue_wraps_around_for_negative_a)
{
  // ColorOKLCh has hue = atan2(b, a), which produces values in (-pi, pi].
  // A color with a negative a (and positive b) yields a hue in the second
  // quadrant, i.e. > pi/2 and < pi.
  ColorOKLCh c(0.5f, 0.1f, 2.0f);  // h = atan2(0.1, 0.5) ≈ 0.197 rad
  EXPECT_LT(c.h, 3.14159265f);
  EXPECT_GT(c.h, 0.0f);
}

/* EOF */
