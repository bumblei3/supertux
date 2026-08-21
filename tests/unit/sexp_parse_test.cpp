// SExp - A S-Expression Parser for C++
// Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//               2015 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <gtest/gtest.h>

#include <sstream>

#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "sexp/parser.hpp"
#include "sexp/value.hpp"

TEST(SexpParseTest, parse_uint32_max)
{
  std::string text = "(big 4294967295)\n";
  std::istringstream in(text);
  sexp::Value sx = sexp::Parser::from_stream(in, true);

  ASSERT_TRUE(sx.is_array());
  auto const& arr = sx.as_array();
  ASSERT_EQ(2u, arr.size());
  ASSERT_TRUE(arr[0].is_symbol());
  ASSERT_EQ("big", arr[0].as_string());
  ASSERT_TRUE(arr[1].is_integer());
  ASSERT_EQ(4294967295LL, arr[1].as_int());
}

// Regression test: sexp-cpp parses UINT32_MAX (4294967295) correctly.
// Before the fix, stoi() threw std::out_of_range because the value exceeds
// INT_MAX (2147483647). sexp-cpp now uses stoll() which handles the full
// long long range.
TEST(SexpParseTest, parse_uint32_max_full_range)
{
  // UINT32_MAX = 2^32 - 1 = 4294967295
  std::string text = "(value 4294967295)\n";
  std::istringstream in(text);
  sexp::Value sx = sexp::Parser::from_stream(in, true);
  ASSERT_TRUE(sx.is_array());
  auto const& arr = sx.as_array();
  ASSERT_EQ(2u, arr.size());
  ASSERT_TRUE(arr[1].is_integer());
  ASSERT_EQ(4294967295LL, arr[1].as_int());
}

// Regression test: as_int() returns long long, not int.
// Before the fix, as_int() returned int which truncated UINT32_MAX to -1.
TEST(SexpParseTest, as_int_returns_long_long)
{
  std::string text = "(val 4294967295)\n";
  std::istringstream in(text);
  sexp::Value sx = sexp::Parser::from_stream(in, true);
  ASSERT_TRUE(sx.is_array());
  auto const& arr = sx.as_array();
  // Verify as_int() returns long long, not int (which would be -1)
  ASSERT_EQ(4294967295LL, arr[1].as_int());
}

TEST(SexpParseTest, readerdocument_autotiles_satc)
{
  // Reproduce the AutotileParser path: ReaderDocument::from_string → get_root() → get_mapping() → get_iter()
  // This is the path that fails in SmokeRenderPixels with "expected integer in expression: #(default 11)"
  std::string text =
    "(supertux-autotiles\n"
    "  (autotileset\n"
    "    (name \"snow\")\n"
    "    (default 11)\n"
    "  )\n"
    ")\n";
  auto doc = ReaderDocument::from_string(text, "<test>");
  auto root = doc.get_root();

  ASSERT_EQ("supertux-autotiles", root.get_name());
  auto mapping = root.get_mapping();
  auto iter = mapping.get_iter();
  ASSERT_TRUE(iter.next());
  ASSERT_EQ("autotileset", iter.get_key());

  auto ats_mapping = iter.as_mapping();
  std::string name;
  ASSERT_TRUE(ats_mapping.get("name", name));
  ASSERT_EQ("snow", name);

  uint32_t default_id = 0;
  ASSERT_TRUE(ats_mapping.get("default", default_id));
  ASSERT_EQ(11u, default_id);
}