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

// Coverage for util/string_util.hpp using the st_assert.hpp harness (no gtest/glm/SDL).
// Focuses on replace_all edge cases (including the empty-needle infinite loop fix)
// and split behaviour not exercised by the existing string_util_test.cpp.
//
// Converted from ST_ASSERT harness to GoogleTest for better failure diagnostics.

#include <gtest/gtest.h>

#include "util/string_util.hpp"

#include <string>
#include <vector>

TEST(StringUtilStTest, replace_all_empty_needle_is_noop)
{
  EXPECT_EQ(StringUtil::replace_all("abc", "", "x"), "abc");
}

TEST(StringUtilStTest, replace_all_basic_replacement)
{
  EXPECT_EQ(StringUtil::replace_all("a.b.c", ".", "/"), "a/b/c");
}

TEST(StringUtilStTest, replace_all_empty_replacement_deletes_needle)
{
  EXPECT_EQ(StringUtil::replace_all("aXXb", "XX", ""), "ab");
}

TEST(StringUtilStTest, split_keeps_empty_middle_field)
{
  std::vector<std::string> out;
  StringUtil::split(out, "x,,y", ',');
  ASSERT_EQ(out.size(), 3u);
  EXPECT_TRUE(out[1].empty());
}

TEST(StringUtilStTest, split_trailing_empty_field)
{
  std::vector<std::string> out;
  StringUtil::split(out, "a,b,", ',');
  ASSERT_EQ(out.size(), 3u);
  EXPECT_EQ("a", out[0]);
  EXPECT_EQ("b", out[1]);
  EXPECT_TRUE(out[2].empty());
}

TEST(StringUtilStTest, split_leading_empty_field)
{
  std::vector<std::string> out;
  StringUtil::split(out, ",b,c", ',');
  ASSERT_EQ(out.size(), 3u);
  EXPECT_TRUE(out[0].empty());
  EXPECT_EQ("b", out[1]);
  EXPECT_EQ("c", out[2]);
}

TEST(StringUtilStTest, split_empty_string_yields_empty)
{
  std::vector<std::string> out;
  StringUtil::split(out, "", ',');
  ASSERT_EQ(out.size(), 1u);
  EXPECT_TRUE(out[0].empty());
}

TEST(StringUtilStTest, split_no_delimiter_returns_whole)
{
  std::vector<std::string> out;
  StringUtil::split(out, "hello", ',');
  ASSERT_EQ(out.size(), 1u);
  EXPECT_EQ("hello", out[0]);
}

TEST(StringUtilStTest, replace_all_consecutive_delimiters)
{
  EXPECT_EQ(StringUtil::replace_all("a,,b,,c", ",,", "|"),
            "a|b|c");
}

TEST(StringUtilStTest, replace_all_overlapping_matches)
{
  // Non-overlapping: "aba", needle "aba" -> "x"
  EXPECT_EQ(StringUtil::replace_all("aba", "aba", "x"), "x");
}

TEST(StringUtilStTest, replace_all_multibyte)
{
  EXPECT_EQ(StringUtil::replace_all("äöü", "ö", "Opcode"),
            "äOpcodeü");
}