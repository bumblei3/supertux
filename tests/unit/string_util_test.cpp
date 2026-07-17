//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include "util/string_util.hpp"

TEST(StringUtilTest, numeric_sort_test)
{
  std::vector<std::string> unsorted_lst =
    {
      "B1235",
      "A123",
      "A123",
      "A12",
      "B12323423A233",
      "B12323423A1231",
      "Z1",
      "A1A123",
      "A1A1",
      "A1A12"
    };

  /* FIXME: this is the result from 'sort -n', which is different from
     what StringUtil::numeric_less produces.
  std::vector<std::string> sorted_lst =
    {
      "A12",
      "A123",
      "A123",
      "A1A1",
      "A1A12"
      "A1A123",
      "B12323423A1231",
      "B12323423A233",
      "B1235",
      "Z1",
    };
  */

  std::vector<std::string> actual_lst =
    {
      "A1A1",
      "A1A12",
      "A1A123",
      "A12",
      "A123",
      "A123",
      "B1235",
      "B12323423A233",
      "B12323423A1231",
      "Z1"
    };

  std::sort(unsorted_lst.begin(), unsorted_lst.end(), StringUtil::numeric_less);

  ASSERT_EQ(actual_lst, unsorted_lst);
}

TEST(StringUtilTest, numeric_less_digit_equal_then_char)
{
  // Same-length numbers that differ in a digit exercise the inner
  // digit-by-digit compare that returns lhs[j] < rhs[j].
  EXPECT_TRUE(StringUtil::numeric_less("12a", "13b"));
  EXPECT_FALSE(StringUtil::numeric_less("13b", "12a"));
  // Same number, same trailing -> size decides.
  EXPECT_TRUE(StringUtil::numeric_less("12", "123"));
  EXPECT_FALSE(StringUtil::numeric_less("123", "12"));
}

TEST(StringUtilTest, has_suffix)
{
  EXPECT_TRUE(StringUtil::has_suffix("foobar.txt", ".txt"));
  EXPECT_FALSE(StringUtil::has_suffix("foobar.txt", ".png"));
  // suffix longer than string -> false
  EXPECT_FALSE(StringUtil::has_suffix("ab", "abcd"));
  // exact match
  EXPECT_TRUE(StringUtil::has_suffix("level.stl", "level.stl"));
  // empty suffix is always a suffix
  EXPECT_TRUE(StringUtil::has_suffix("anything", ""));
}

TEST(StringUtilTest, starts_with)
{
  EXPECT_TRUE(StringUtil::starts_with("foobar", "foo"));
  EXPECT_FALSE(StringUtil::starts_with("foobar", "bar"));
  EXPECT_FALSE(StringUtil::starts_with("ab", "abcd"));
  EXPECT_TRUE(StringUtil::starts_with("level.stl", "level.stl"));
  // empty prefix always matches
  EXPECT_TRUE(StringUtil::starts_with("anything", ""));
}

TEST(StringUtilTest, tolower)
{
  EXPECT_EQ(StringUtil::tolower("FooBar123"), "foobar123");
  EXPECT_EQ(StringUtil::tolower("ALREADY"), "already");
  EXPECT_EQ(StringUtil::tolower(""), "");
  EXPECT_EQ(StringUtil::tolower("MiXeD cAsE!"), "mixed case!");
}

TEST(StringUtilTest, replace_all_basic)
{
  EXPECT_EQ(StringUtil::replace_all("a.b.c", ".", "/"), "a/b/c");
  EXPECT_EQ(StringUtil::replace_all("hello world", "o", "0"), "hell0 w0rld");
  EXPECT_EQ(StringUtil::replace_all("no match here", "xyz", "abc"), "no match here");
}

TEST(StringUtilTest, replace_all_empty_needle_is_noop)
{
  // An empty needle would otherwise trigger an infinite loop in the
  // naive find/replace. Guard must return the input unchanged.
  EXPECT_EQ(StringUtil::replace_all("anything", "", "X"), "anything");
  EXPECT_EQ(StringUtil::replace_all("", "", "X"), "");
}

TEST(StringUtilTest, replace_all_overlapping_avoided)
{
  // The scan position advances past the replacement, so "aa" -> "ba"
  // applied to "aaa" yields "baa", not an infinite expansion.
  EXPECT_EQ(StringUtil::replace_all("aaa", "aa", "b"), "ba");
  EXPECT_EQ(StringUtil::replace_all("aaaa", "aa", "b"), "bb");
}

TEST(StringUtilTest, split)
{
  std::vector<std::string> out;
  StringUtil::split(out, "a,b,c", ',');
  ASSERT_EQ(out, (std::vector<std::string>{"a", "b", "c"}));

  out.clear();
  StringUtil::split(out, "single", ',');
  ASSERT_EQ(out, (std::vector<std::string>{"single"}));

  out.clear();
  StringUtil::split(out, "x::y::z", ':');
  ASSERT_EQ(out, (std::vector<std::string>{"x", "", "y", "", "z"}));

  out.clear();
  StringUtil::split(out, "", ',');
  // A stringstream over "" is immediately at EOF, so nothing is pushed.
  ASSERT_EQ(out, (std::vector<std::string>{}));
}

/* EOF */
