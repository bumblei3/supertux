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

#include "st_assert.hpp"
#include "util/colorspace_oklab.hpp"
#include "video/color.hpp"

#include <cmath>

namespace {

bool approx(float a, float b, float eps = 0.01f)
{
  return std::fabs(a - b) < eps;
}

} // namespace

int main(void)
{
  // White (1,1,1) maps to OKLab lightness L = 1.0 and (near) zero chroma.
  {
    ColorOKLCh w(Color(1.0f, 1.0f, 1.0f));
    ST_ASSERT("white: L approx 1.0", approx(w.L, 1.0f));
    ST_ASSERT("white: chroma approx 0", w.C < 0.01f);
  }

  // Black (0,0,0) maps to L = 0, C = 0, and greyscale forces h = 0.
  {
    ColorOKLCh b(Color(0.0f, 0.0f, 0.0f));
    ST_ASSERT("black: L approx 0", approx(b.L, 0.0f));
    ST_ASSERT("black: chroma approx 0", b.C < 0.01f);
    ST_ASSERT("black: hue forced to 0", b.h == 0.0f);
  }

  // Mid grey: chroma stays ~0 and the greyscale guard forces h to exactly 0.
  {
    ColorOKLCh g(Color(0.5f, 0.5f, 0.5f));
    ST_ASSERT("grey: chroma approx 0", g.C < 0.01f);
    ST_ASSERT("grey: hue forced to 0", g.h == 0.0f);
    // Lightness must sit strictly between black and white.
    ST_ASSERT("grey: 0 < L < 1", g.L > 0.0f && g.L < 1.0f);
  }

  // Pure red has substantial chroma (reference L ~= 0.628, C ~= 0.258).
  {
    ColorOKLCh r(Color(1.0f, 0.0f, 0.0f));
    ST_ASSERT("red: L approx 0.628", approx(r.L, 0.628f, 0.02f));
    ST_ASSERT("red: chroma is large", r.C > 0.2f);
  }

  // Lightness is monotonic in a greyscale ramp: darker input -> smaller L.
  {
    ColorOKLCh dark(Color(0.2f, 0.2f, 0.2f));
    ColorOKLCh light(Color(0.8f, 0.8f, 0.8f));
    ST_ASSERT("greyscale ramp: L monotonic", dark.L < light.L);
  }

  // get_modified_lightness: for L = 0 it must return 0 (the estimate is
  // anchored at black), and it is monotonic increasing in L.
  {
    ColorOKLCh black(Color(0.0f, 0.0f, 0.0f));
    ColorOKLCh white(Color(1.0f, 1.0f, 1.0f));
    float lm_black = black.get_modified_lightness();
    float lm_white = white.get_modified_lightness();
    ST_ASSERT("modified lightness: black approx 0", approx(lm_black, 0.0f));
    ST_ASSERT("modified lightness: white > black", lm_white > lm_black);
  }

  // The explicit (L, C, h) constructor stores its arguments verbatim.
  {
    ColorOKLCh c(0.5f, 0.1f, 1.2f);
    ST_ASSERT("explicit ctor: L", c.L == 0.5f);
    ST_ASSERT("explicit ctor: C", c.C == 0.1f);
    ST_ASSERT("explicit ctor: h", c.h == 1.2f);
  }
}

/* EOF */
