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

// Full coverage of util/reader_error.hpp: every assert_* helper family
// (boolean, integer, real, symbol, string, array, array_size_eq, array_size_ge)
// is exercised for both the success and the failure path, and the error
// message is verified to carry file:line + expression context.

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include <sexp/value.hpp>
#include <sexp/io.hpp>

#include "util/reader_document.hpp"
#include "util/reader_error.hpp"

namespace {

// Builds a reader document whose root contains named sexp::Values we can
// pluck out by name (a convenience for the error-path tests).
ReaderDocument make_doc()
{
  std::istringstream in(
    "(test-root\n"
    "   (a-boolean #t)\n"
    "   (an-integer 42)\n"
    "   (a-real 3.14)\n"
    "   (a-symbol mykey)\n"
    "   (a-string \"hello\")\n"
    "   (an-array (x 1) (y 2))\n"
    ")\n");
  return ReaderDocument::from_stream(in);
}

// Returns the sexp child whose first element is the given symbol.
sexp::Value find_value(ReaderDocument const& doc, const char* name)
{
  for (auto const& child : doc.get_root().get_sexp().as_array()) {
    if (!child.is_array() || child.as_array().empty())
      continue;
    auto const& arr = child.as_array();
    if (arr[0].is_symbol() && arr[0].as_string() == name)
      return arr[1];   // der Wert (Index 1), nicht das ganze Array
  }
  return sexp::Value::nil();
}

} // namespace

// --- assert_is_boolean -------------------------------------------------

TEST(ReaderErrorTest, assert_is_boolean_accepts_boolean)
{
  auto doc = make_doc();
  auto sx = find_value(doc, "a-boolean");
  ASSERT_TRUE(sx.is_boolean());
  ASSERT_NO_THROW({ assert_is_boolean(doc, sx); });
}

TEST(ReaderErrorTest, assert_is_boolean_rejects_non_boolean)
{
  auto doc = make_doc();
  auto sx = find_value(doc, "an-integer"); // integer, not boolean
  EXPECT_THROW({ assert_is_boolean(doc, sx); }, std::runtime_error);
}

// --- assert_is_integer -------------------------------------------------

TEST(ReaderErrorTest, assert_is_integer_accepts_integer)
{
  auto doc = make_doc();
  auto sx = find_value(doc, "an-integer");
  ASSERT_TRUE(sx.is_integer());
  ASSERT_NO_THROW({ assert_is_integer(doc, sx); });
}

TEST(ReaderErrorTest, assert_is_integer_rejects_non_integer)
{
  auto doc = make_doc();
  auto sx = find_value(doc, "a-boolean"); // boolean, not integer
  EXPECT_THROW({ assert_is_integer(doc, sx); }, std::runtime_error);
}

// --- assert_is_real ----------------------------------------------------

TEST(ReaderErrorTest, assert_is_real_accepts_real)
{
  auto doc = make_doc();
  auto sx = find_value(doc, "a-real");
  ASSERT_TRUE(sx.is_real());
  ASSERT_NO_THROW({ assert_is_real(doc, sx); });
}

TEST(ReaderErrorTest, assert_is_real_rejects_non_real)
{
  auto doc = make_doc();
  // sexp-cpp stellt ganze Zahlen meist als Real dar (is_real() == true für
  // ganze Zahlen). Um einen echten Non-Real zu bekommen, wähle ein Symbol
  // oder einen Boolean, nicht einen Integer.
  auto sx = find_value(doc, "a-symbol");
  EXPECT_THROW({ assert_is_real(doc, sx); }, std::runtime_error);
}

// --- assert_is_symbol -------------------------------------------------

TEST(ReaderErrorTest, assert_is_symbol_accepts_symbol)
{
  auto doc = make_doc();
  auto sx = find_value(doc, "a-symbol");
  ASSERT_TRUE(sx.is_symbol());
  ASSERT_NO_THROW({ assert_is_symbol(doc, sx); });
}

TEST(ReaderErrorTest, assert_is_symbol_rejects_non_symbol)
{
  auto doc = make_doc();
  // Ein Integer ist kein Symbol.
  auto sx = find_value(doc, "an-integer");
  EXPECT_THROW({ assert_is_symbol(doc, sx); }, std::runtime_error);
}

// --- assert_is_string -------------------------------------------------

TEST(ReaderErrorTest, assert_is_string_accepts_string)
{
  // make_doc() enthält (a-string "hello") — find_value liefert den Wert
  // (Index 1 des Arrays), also den String "hello" direkt.
  auto doc = make_doc();
  auto sx = find_value(doc, "a-string");
  ASSERT_TRUE(sx.is_string()) << "expected string value, got " << sx;
  ASSERT_NO_THROW({ assert_is_string(doc, sx); });
}

TEST(ReaderErrorTest, assert_is_string_rejects_non_string)
{
  auto doc = make_doc();
  auto sx = find_value(doc, "an-integer"); // integer, not string
  EXPECT_THROW({ assert_is_string(doc, sx); }, std::runtime_error);
}

// --- assert_is_array -------------------------------------------------

TEST(ReaderErrorTest, assert_is_array_accepts_array)
{
  auto doc = make_doc();
  auto sx = find_value(doc, "an-array");
  ASSERT_TRUE(sx.is_array());
  ASSERT_NO_THROW({ assert_is_array(doc, sx); });
}

TEST(ReaderErrorTest, assert_is_array_rejects_non_array)
{
  auto doc = make_doc();
  auto sx = find_value(doc, "a-boolean"); // boolean, not array
  EXPECT_THROW({ assert_is_array(doc, sx); }, std::runtime_error);
}

// --- assert_array_size_eq ---------------------------------------------

TEST(ReaderErrorTest, assert_array_size_eq_passes_on_exact_match)
{
  auto doc = make_doc();
  auto root = doc.get_root().get_sexp();
  int const size = static_cast<int>(root.as_array().size());
  ASSERT_NO_THROW({ assert_array_size_eq(doc, root, size); });
}

TEST(ReaderErrorTest, assert_array_size_eq_throws_on_mismatch)
{
  auto doc = make_doc();
  auto root = doc.get_root().get_sexp();
  int const wrong_size = static_cast<int>(root.as_array().size()) + 5;
  try {
    assert_array_size_eq(doc, root, wrong_size);
    FAIL() << "expected std::runtime_error";
  } catch (std::runtime_error const& err) {
    std::string const msg = err.what();
    EXPECT_NE(msg.find("elements"), std::string::npos);
    EXPECT_NE(msg.find("must have"), std::string::npos);
  }
}

TEST(ReaderErrorTest, assert_array_size_eq_error_message_contains_context)
{
  auto doc = make_doc();
  auto sx = find_value(doc, "a-boolean"); // not an array
  try {
    assert_array_size_eq(doc, sx, 1);
    FAIL() << "expected std::runtime_error";
  } catch (std::runtime_error const& err) {
    std::string const msg = err.what();
    // Die reale raise_exception_real emittiert:
    //   [filename:line] doc.get_filename():sx.get_line(): usermsg in expression:\n    sx
    // Also erwarte ich den Source-Hinweis und die Typ-Beschreibung.
    EXPECT_NE(msg.find("reader_error.hpp:"), std::string::npos);
    EXPECT_NE(msg.find("expected array"), std::string::npos);
    EXPECT_NE(msg.find("in expression:"), std::string::npos);
  }
}

// --- assert_array_size_ge ---------------------------------------------

TEST(ReaderErrorTest, assert_array_size_ge_passes_on_larger)
{
  auto doc = make_doc();
  auto root = doc.get_root().get_sexp();
  ASSERT_NO_THROW({ assert_array_size_ge(doc, root, 2); });
}

TEST(ReaderErrorTest, assert_array_size_ge_passes_on_exact)
{
  auto doc = make_doc();
  auto root = doc.get_root().get_sexp();
  int const size = static_cast<int>(root.as_array().size());
  ASSERT_NO_THROW({ assert_array_size_ge(doc, root, size); });
}

TEST(ReaderErrorTest, assert_array_size_ge_throws_on_smaller)
{
  auto doc = make_doc();
  auto root = doc.get_root().get_sexp();
  int const too_big = static_cast<int>(root.as_array().size()) + 100;
  EXPECT_THROW({ assert_array_size_ge(doc, root, too_big); }, std::runtime_error);
}

// --- error message format (cross-cutting) -----------------------------

TEST(ReaderErrorTest, any_type_error_carries_source_location)
{
  auto doc = make_doc();
  auto sx = find_value(doc, "a-boolean"); // wrong type for integer assertion

  try {
    assert_is_integer(doc, sx);
    FAIL() << "expected std::runtime_error";
  } catch (std::runtime_error const& err) {
    std::string const msg = err.what();
    // Die reale raise_exception_real embeddert:
    //   [filename:line] doc.get_filename():sx.get_line(): usermsg in expression:\n    sx
    EXPECT_NE(msg.find("reader_error.hpp:"), std::string::npos);
    EXPECT_NE(msg.find("expected integer"), std::string::npos);
    EXPECT_NE(msg.find("in expression:"), std::string::npos);
  }
}
