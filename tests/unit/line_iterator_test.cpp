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

// Dependency-free coverage for util/line_iterator.hpp using the st_assert
// harness (no gtest/SDL). LineIterator splits a string at '\n': a single
// trailing newline is ignored, but interior/leading empty lines are returned.

#include "st_assert.hpp"
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

int main(void)
{
  // Empty input yields no lines at all.
  {
    auto lines = split_lines("");
    ST_ASSERT("empty string produces no lines", lines.empty());
  }

  // A single line without a trailing newline is returned verbatim.
  {
    auto lines = split_lines("Hello");
    ST_ASSERT("single line: one entry", lines.size() == 1);
    ST_ASSERT("single line: content", lines.size() == 1 && lines[0] == "Hello");
  }

  // Two lines separated by a newline; newline itself is not included.
  {
    auto lines = split_lines("Hello\nWorld");
    ST_ASSERT("two lines: count", lines.size() == 2);
    ST_ASSERT("two lines: first", lines.size() == 2 && lines[0] == "Hello");
    ST_ASSERT("two lines: second", lines.size() == 2 && lines[1] == "World");
  }

  // A single trailing newline: the class doc claims a trailing newline is
  // "ignored", but the implementation only does so when the whole line before
  // it is empty (see the "\n" case below). For "Hello\n" it actually yields a
  // trailing empty line. We pin the ACTUAL behaviour here as a regression
  // baseline; the doc/impl mismatch is a known quirk of LineIterator::next().
  {
    auto lines = split_lines("Hello\n");
    ST_ASSERT("trailing newline: yields two entries (quirk)", lines.size() == 2);
    ST_ASSERT("trailing newline: first is content",
              lines.size() == 2 && lines[0] == "Hello");
    ST_ASSERT("trailing newline: second is empty",
              lines.size() == 2 && lines[1].empty());
  }

  // An interior empty line (two newlines in a row) IS returned as "".
  {
    auto lines = split_lines("a\n\nb");
    ST_ASSERT("interior empty line: count", lines.size() == 3);
    ST_ASSERT("interior empty line: a", lines.size() == 3 && lines[0] == "a");
    ST_ASSERT("interior empty line: empty", lines.size() == 3 && lines[1].empty());
    ST_ASSERT("interior empty line: b", lines.size() == 3 && lines[2] == "b");
  }

  // A lone newline: here the trailing-newline guard DOES fire, so no lines are
  // produced at all (the one case where "trailing newline ignored" holds).
  {
    auto lines = split_lines("\n");
    ST_ASSERT("lone newline: produces no lines", lines.empty());
  }

  // next() must be idempotent once exhausted (keeps returning false).
  {
    std::string s = "x";
    LineIterator it(s);
    ST_ASSERT("first next() true", it.next());
    ST_ASSERT("second next() false", !it.next());
    ST_ASSERT("third next() still false", !it.next());
  }
}

/* EOF */
