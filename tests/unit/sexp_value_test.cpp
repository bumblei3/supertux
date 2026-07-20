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
//  along with this program.  if not, see <http://www.gnu.org/licenses/>.

// Round-trip test for the sexp::Value parser/serializer. SuperTux's
// save/load (Writer/Reader) ultimately produces and consumes sexp
// values; if a Value serializes and re-parses to a different string, the
// on-disk format drifts and old saves can fail to load. This pins the
// canonical text form of the value types the engine actually writes
// (int, bool, string, symbol, nested list, array).

#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include <sexp/parser.hpp>
#include <sexp/value.hpp>
#include <sexp/io.hpp>

namespace {

// Serialize a value to its canonical string form.
std::string to_string(const sexp::Value& v)
{
  std::ostringstream os;
  os << v;
  return os.str();
}

// Round-trip: serialize, parse back, serialize again -- the two strings
// must be identical (canonical form is stable).
std::string roundtrip(const sexp::Value& v)
{
  const std::string first = to_string(v);
  const sexp::Value reparsed = sexp::Parser::from_string(first);
  const std::string second = to_string(reparsed);
  return second;
}

} // namespace

TEST(SexpValueTest, integer_roundtrip)
{
  const sexp::Value v = sexp::Value::integer(123456789);
  EXPECT_EQ(to_string(v), "123456789");
  EXPECT_EQ(roundtrip(v), "123456789");
}

TEST(SexpValueTest, boolean_roundtrip)
{
  EXPECT_EQ(to_string(sexp::Value::boolean(true)), "#t");
  EXPECT_EQ(to_string(sexp::Value::boolean(false)), "#f");
  EXPECT_EQ(roundtrip(sexp::Value::boolean(true)), "#t");
  EXPECT_EQ(roundtrip(sexp::Value::boolean(false)), "#f");
}

TEST(SexpValueTest, string_roundtrip)
{
  const sexp::Value v = sexp::Value::string("Hello World");
  EXPECT_EQ(to_string(v), "\"Hello World\"");
  EXPECT_EQ(roundtrip(v), "\"Hello World\"");
}

TEST(SexpValueTest, symbol_roundtrip)
{
  const sexp::Value v = sexp::Value::symbol("supertux-test");
  EXPECT_EQ(to_string(v), "supertux-test");
  EXPECT_EQ(roundtrip(v), "supertux-test");
}

TEST(SexpValueTest, nested_list_roundtrip)
{
  // (mymapping (a 1) (b 2))
  const sexp::Value v =
    sexp::Value::list(sexp::Value::symbol("mymapping"),
                      sexp::Value::list(sexp::Value::symbol("a"),
                                        sexp::Value::integer(1)),
                      sexp::Value::list(sexp::Value::symbol("b"),
                                        sexp::Value::integer(2)));
  const std::string expected = "(mymapping (a 1) (b 2))";
  EXPECT_EQ(to_string(v), expected);
  EXPECT_EQ(roundtrip(v), expected);
}

TEST(SexpValueTest, array_roundtrip)
{
  const sexp::Value v =
    sexp::Value::array({sexp::Value::integer(5), sexp::Value::integer(5),
                        sexp::Value::integer(4), sexp::Value::integer(4)});
  const std::string expected = "#(5 5 4 4)";
  EXPECT_EQ(to_string(v), expected);
  EXPECT_EQ(roundtrip(v), expected);
}

/* EOF */
