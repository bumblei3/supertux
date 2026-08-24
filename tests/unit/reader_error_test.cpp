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
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Coverage for util/reader_error.hpp: the type-assertion helpers every
// level/worldmap parser flows through. Header-only; needs sexp + reader
// document (same link setup as ReaderTest, via its stub).

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include <sexp/value.hpp>
#include <sexp/io.hpp>

#include "util/reader_document.hpp"
#include "util/reader_error.hpp"

namespace {

ReaderDocument make_doc()
{
  std::istringstream in("(test-root (value #t) (num 42))\n");
  return ReaderDocument::from_stream(in);
}

sexp::Value find_value(ReaderDocument const& doc, const char* name)
{
  for (auto const& child : doc.get_root().get_sexp().as_array())
  {
    if (!child.is_array() || child.as_array().empty())
      continue;
    auto const& arr = child.as_array();
    if (arr[0].is_symbol() && arr[0].as_string() == name)
      return arr[1];
  }
  return sexp::Value::nil();
}

} // namespace

TEST(ReaderErrorTest, assert_is_boolean_accepts_boolean)
{
  auto doc = make_doc();
  auto sx = find_value(doc, "value");
  ASSERT_NO_THROW({ assert_is_boolean(doc, sx); });
}

TEST(ReaderErrorTest, assert_is_integer_rejects_non_integer)
{
  auto doc = make_doc();
  auto sx = find_value(doc, "value"); // boolean
  EXPECT_THROW({ assert_is_integer(doc, sx); }, std::runtime_error);
}

TEST(ReaderErrorTest, assert_is_string_error_message_contains_context)
{
  auto doc = make_doc();
  auto sx = doc.get_root().get_sexp(); // root array, not a string
  try
  {
    assert_is_string(doc, sx);
    FAIL() << "expected std::runtime_error";
  }
  catch (std::runtime_error const& err)
  {
    std::string const msg = err.what();
    // The message must carry file:line context and the failing expression.
    EXPECT_NE(msg.find("expected string"), std::string::npos);
    EXPECT_NE(msg.find("test-root"), std::string::npos);
    EXPECT_NE(msg.find("reader_error.hpp:"), std::string::npos); // raise_exception source location
    EXPECT_NE(msg.find("<stream>"), std::string::npos); // document filename
  }
}

TEST(ReaderErrorTest, assert_array_size_eq_passes_on_exact_match)
{
  auto doc = make_doc();
  // Root itself is an array with several children.
  auto root = doc.get_root().get_sexp();
  int const size = static_cast<int>(root.as_array().size());
  ASSERT_NO_THROW({ assert_array_size_eq(doc, root, size); });
}

TEST(ReaderErrorTest, assert_array_size_eq_throws_on_mismatch)
{
  auto doc = make_doc();
  auto root = doc.get_root().get_sexp();
  int const wrong_size = static_cast<int>(root.as_array().size()) + 5;
  try
  {
    assert_array_size_eq(doc, root, wrong_size);
    FAIL() << "expected std::runtime_error";
  }
  catch (std::runtime_error const& err)
  {
    std::string const msg = err.what();
    EXPECT_NE(msg.find("elements"), std::string::npos);
  }
}

TEST(ReaderErrorTest, assert_array_size_ge_passes_on_larger)
{
  auto doc = make_doc();
  auto root = doc.get_root().get_sexp();
  ASSERT_NO_THROW({ assert_array_size_ge(doc, root, 2); });
}

TEST(ReaderErrorTest, assert_array_size_ge_throws_on_smaller)
{
  auto doc = make_doc();
  auto root = doc.get_root().get_sexp();
  int const huge = static_cast<int>(root.as_array().size()) + 100;
  EXPECT_THROW({ assert_array_size_ge(doc, root, huge); }, std::runtime_error);
}

/* EOF */
