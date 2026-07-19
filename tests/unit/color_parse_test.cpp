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

// Coverage for video/color.hpp string (de)serialization: deserialize_from_hex,
// deserialize_from_rgb, and the serialize/deserialize round-trips. Verifies
// valid parsing, out-of-range rejection, and malformed-input handling.
// Logging stubbed (color_parse_test_stub.cpp) to stay engine-free.

#include "st_assert.hpp"
#include "video/color.hpp"

#include <optional>

namespace {

bool approx(float a, float b, float eps = 0.004f)
{
  float d = a - b;
  return (d < 0 ? -d : d) < eps;
}

} // namespace

int main(void)
{
  // deserialize_from_hex: valid 6-digit hex maps to the expected channels.
  {
    auto c = Color::deserialize_from_hex("#ff0000");
    ST_ASSERT("hex red present", c.has_value());
    ST_ASSERT("hex red.r", approx(c->red, 1.0f));
    ST_ASSERT("hex red.g", approx(c->green, 0.0f));
    ST_ASSERT("hex red.b", approx(c->blue, 0.0f));
    ST_ASSERT("hex red.a", approx(c->alpha, 1.0f));
  }

  {
    auto c = Color::deserialize_from_hex("#00ff00");
    ST_ASSERT("hex green present", c.has_value());
    ST_ASSERT("hex green.g", approx(c->green, 1.0f));
  }

  {
    auto c = Color::deserialize_from_hex("#0000ff");
    ST_ASSERT("hex blue present", c.has_value());
    ST_ASSERT("hex blue.b", approx(c->blue, 1.0f));
  }

  // Mid grey #808080 -> each channel ~0.5.
  {
    auto c = Color::deserialize_from_hex("#808080");
    ST_ASSERT("hex grey present", c.has_value());
    ST_ASSERT("hex grey ~0.5", approx(c->red, 0.5f) && approx(c->green, 0.5f) && approx(c->blue, 0.5f));
  }

  // Whitespace around the value is accepted.
  {
    auto c = Color::deserialize_from_hex("  #ffffff  ");
    ST_ASSERT("hex with whitespace", c.has_value());
    ST_ASSERT("hex white all 1", approx(c->red, 1.0f) && approx(c->green, 1.0f) && approx(c->blue, 1.0f));
  }

  // serialize_to_hex round-trips back to the same hex string.
  // Note: channels are truncated (not rounded), so 0.5f -> 127 = 0x7f.
  {
    ST_ASSERT("hex round-trip",
              Color::serialize_to_hex(Color(1.0f, 0.0f, 0.5f)) == "#FF007F");
  }

  // Malformed hex inputs must be rejected (nullopt).
  {
    ST_ASSERT("hex: empty -> nullopt", !Color::deserialize_from_hex("").has_value());
    ST_ASSERT("hex: 3-digit -> nullopt", !Color::deserialize_from_hex("#f00").has_value());
    ST_ASSERT("hex: 8-digit -> nullopt", !Color::deserialize_from_hex("#ff0000ff").has_value());
    ST_ASSERT("hex: non-hex -> nullopt", !Color::deserialize_from_hex("#zzzzzz").has_value());
    ST_ASSERT("hex: no-hash -> nullopt", !Color::deserialize_from_hex("ff0000").has_value());
    ST_ASSERT("hex: too short -> nullopt", !Color::deserialize_from_hex("#ff").has_value());
  }

  // deserialize_from_rgb: valid rgb(r,g,b) with 0..255 values.
  {
    auto c = Color::deserialize_from_rgb("rgb(255, 128, 0)");
    ST_ASSERT("rgb present", c.has_value());
    ST_ASSERT("rgb r", approx(c->red, 1.0f));
    ST_ASSERT("rgb g", approx(c->green, 128.0f / 255.0f));
    ST_ASSERT("rgb b", approx(c->blue, 0.0f));
  }

  // serialize_to_rgb round-trips the same value back.
  // Channels are truncated, so 0.5f -> 127.
  {
    ST_ASSERT("rgb round-trip",
              Color::serialize_to_rgb(Color(1.0f, 0.5f, 0.0f)) == "rgb(255,127,0)");
  }

  // Out-of-range and malformed rgb inputs are rejected.
  {
    ST_ASSERT("rgb: 256 -> nullopt", !Color::deserialize_from_rgb("rgb(256,0,0)").has_value());
    ST_ASSERT("rgb: -1 -> nullopt", !Color::deserialize_from_rgb("rgb(-1,0,0)").has_value());
    ST_ASSERT("rgb: missing parens -> nullopt", !Color::deserialize_from_rgb("255,0,0").has_value());
    ST_ASSERT("rgb: empty -> nullopt", !Color::deserialize_from_rgb("").has_value());
    ST_ASSERT("rgb: non-numeric -> nullopt", !Color::deserialize_from_rgb("rgb(a,b,c)").has_value());
  }
}

/* EOF */
