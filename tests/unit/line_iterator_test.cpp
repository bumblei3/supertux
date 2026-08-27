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

// Coverage for util/line_iterator.hpp: LineIterator splits a string at '\n'.
// A single trailing newline is ignored, but interior/leading empty lines are
// returned. We pin the actual implementation behaviour (the doc/impl mismatch
// on trailing newlines is documented below).
//
// Converted from ST_ASSERT harness to GoogleTest for better failure diagnostics.

#include <gtest/gtest.h>

#include "util/line_iterator.hpp"

#include <string>
#include <vector>

namespace {

// Collect all lines produced by iterating over the given input.
std::vector<std::string> split_lines(const std::string& input)
{
  std::vector<std::string> out;
  LineIterator it(input);
  while (it.next())
    out.push_back(it.get());
  return out;
}

} // namespace

TEST(LineIteratorTest, empty_input_produces_no_lines)
{
  EXPECT_TRUE(split_lines("").empty());
}

TEST(LineIteratorTest, single_line_without_trailing_newline)
{
  std::vector<std::string> lines = split_lines("Hello");
  ASSERT_EQ(lines.size(), 1u);
  EXPECT_EQ(lines[0], "Hello");
}

TEST(LineIteratorTest, two_lines_separated_by_newline)
{
  std::vector<std::string> lines = split_lines("Hello\nWorld");
  ASSERT_EQ(lines.size(), 2u);
  EXPECT_EQ(lines[0], "Hello");
  EXPECT_EQ(lines[1], "World");
}

// --- Trailing newline quirk (pinned as regression baseline) --------------

// The class doc claims a trailing newline is "ignored", but LineIterator::next()
// only does so when the whole line before it is empty (the lone-"\n" case).
// For "Hello\n" it actually yields a trailing empty line. We pin the ACTUAL
// behaviour here; the doc/impl mismatch is a known quirk.
TEST(LineIteratorTest, trailing_newline_yields_trailing_empty_line)
{
  std::vector<std::string> lines = split_lines("Hello\n");
  ASSERT_EQ(lines.size(), 2u);
  EXPECT_EQ(lines[0], "Hello");
  EXPECT_TRUE(lines[1].empty());
}

// --- Interior empty lines -------------------------------------------------

TEST(LineIteratorTest, interior_empty_line_is_returned)
{
  std::vector<std::string> lines = split_lines("a\n\nb");
  ASSERT_EQ(lines.size(), 3u);
  EXPECT_EQ(lines[0], "a");
  EXPECT_TRUE(lines[1].empty());
  EXPECT_EQ(lines[2], "b");
}

// --- Lone newline: the one case where trailing-newline-is-ignored holds ---

TEST(LineIteratorTest, lone_newline_produces_no_lines)
{
  EXPECT_TRUE(split_lines("\n").empty());
}

// --- next() is idempotent once exhausted ---------------------------------

TEST(LineIteratorTest, next_idempotent_when_exhausted)
{
  std::string s = "x";
  LineIterator it(s);
  EXPECT_TRUE(it.next());
  EXPECT_FALSE(it.next());
  EXPECT_FALSE(it.next());
}