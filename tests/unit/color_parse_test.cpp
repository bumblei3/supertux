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

#include <gtest/gtest.h>

#include "video/color.hpp"

#include <cmath>

namespace {

constexpr float EPS = 0.004f;

bool approx(float a, float b)
{
  return std::fabs(a - b) < EPS;
}

} // namespace

// --- deserialize_from_hex -------------------------------------------------

TEST(ColorParseTest, hex_red_parsed_correctly)
{
  auto c = Color::deserialize_from_hex("#ff0000");
  ASSERT_TRUE(c.has_value());
  EXPECT_NEAR(c->red, 1.0f, EPS);
  EXPECT_NEAR(c->green, 0.0f, EPS);
  EXPECT_NEAR(c->blue, 0.0f, EPS);
  EXPECT_NEAR(c->alpha, 1.0f, EPS);
}

TEST(ColorParseTest, hex_green_parsed_correctly)
{
  auto c = Color::deserialize_from_hex("#00ff00");
  ASSERT_TRUE(c.has_value());
  EXPECT_NEAR(c->green, 1.0f, EPS);
}

TEST(ColorParseTest, hex_blue_parsed_correctly)
{
  auto c = Color::deserialize_from_hex("#0000ff");
  ASSERT_TRUE(c.has_value());
  EXPECT_NEAR(c->blue, 1.0f, EPS);
}

TEST(ColorParseTest, hex_mid_grey_parsed_correctly)
{
  auto c = Color::deserialize_from_hex("#808080");
  ASSERT_TRUE(c.has_value());
  EXPECT_TRUE(approx(c->red, 0.5f));
  EXPECT_TRUE(approx(c->green, 0.5f));
  EXPECT_TRUE(approx(c->blue, 0.5f));
}

TEST(ColorParseTest, hex_whitespace_accepted)
{
  auto c = Color::deserialize_from_hex("  #ffffff  ");
  ASSERT_TRUE(c.has_value());
  EXPECT_TRUE(approx(c->red, 1.0f));
  EXPECT_TRUE(approx(c->green, 1.0f));
  EXPECT_TRUE(approx(c->blue, 1.0f));
}

TEST(ColorParseTest, hex_serialize_roundtrip)
{
  // Channels are truncated (not rounded), so 0.5f -> 127 = 0x7f.
  EXPECT_EQ(Color::serialize_to_hex(Color(1.0f, 0.0f, 0.5f)), "#FF007F");
}

TEST(ColorParseTest, hex_malformed_inputs_rejected)
{
  EXPECT_FALSE(Color::deserialize_from_hex("").has_value());
  EXPECT_FALSE(Color::deserialize_from_hex("#f00").has_value());
  EXPECT_FALSE(Color::deserialize_from_hex("#ff0000ff").has_value());
  EXPECT_FALSE(Color::deserialize_from_hex("#zzzzzz").has_value());
  EXPECT_FALSE(Color::deserialize_from_hex("ff0000").has_value());
  EXPECT_FALSE(Color::deserialize_from_hex("#ff").has_value());
}

// --- deserialize_from_rgb -------------------------------------------------

TEST(ColorParseTest, rgb_parsed_correctly)
{
  auto c = Color::deserialize_from_rgb("rgb(255, 128, 0)");
  ASSERT_TRUE(c.has_value());
  EXPECT_NEAR(c->red, 1.0f, EPS);
  EXPECT_NEAR(c->green, 128.0f / 255.0f, EPS);
  EXPECT_NEAR(c->blue, 0.0f, EPS);
}

TEST(ColorParseTest, rgb_serialize_roundtrip)
{
  // Channels are truncated, so 0.5f -> 127.
  EXPECT_EQ(Color::serialize_to_rgb(Color(1.0f, 0.5f, 0.0f)), "rgb(255,127,0)");
}

TEST(ColorParseTest, rgb_out_of_range_and_malformed_rejected)
{
  EXPECT_FALSE(Color::deserialize_from_rgb("rgb(256,0,0)").has_value());
  EXPECT_FALSE(Color::deserialize_from_rgb("rgb(-1,0,0)").has_value());
  EXPECT_FALSE(Color::deserialize_from_rgb("255,0,0").has_value());
  EXPECT_FALSE(Color::deserialize_from_rgb("").has_value());
  EXPECT_FALSE(Color::deserialize_from_rgb("rgb(a,b,c)").has_value());
}