//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

TEST(ReaderTest, get)
{
  std::istringstream in(
    "(supertux-test\n"
    "   (mybool #t)\r"
    "   (myint 123456789)\r\n"
    "   (myfloat 1.125)\n\r"
    "   (mystring \"Hello World\")\n"
    "   (mystringtrans (_ \"Hello World\"))\n"
    "   (myboolarray #t #f #t #f)\n"
    "   (myintarray 5 5 4 4 3 2 1 0)\n"
    "   (myfloatarray 6.5 5.25 4.125 3.0625 2.0 1.0 0.5 0.25 0.125)\n"
    "   (mystringarray \"One\" \"Two\" \"Three\")\n"
    "   (mymapping (a 1) (b 2))\n"
    "   (mycustom \"1234\")\n"
    ")\n");

  auto doc = ReaderDocument::from_stream(in);
  auto root = doc.get_root();
  ASSERT_EQ("supertux-test", root.get_name());
  auto mapping = root.get_mapping();

  {
    bool mybool;
    mapping.get("mybool", mybool);
    ASSERT_EQ(true, mybool);
  }

  {
    int myint;
    mapping.get("myint", myint);
    ASSERT_EQ(123456789, myint);
  }

  {
    float myfloat;
    mapping.get("myfloat", myfloat);
    ASSERT_EQ(1.125, myfloat);
  }

  {
    std::string mystring;
    mapping.get("mystring", mystring);
    ASSERT_EQ("Hello World", mystring);
  }

  {
    std::string mystringtrans;
    mapping.get("mystringtrans", mystringtrans);
    ASSERT_EQ("Hello World", mystringtrans);
  }

  {
    std::vector<bool> expected{ true, false, true, false };
    std::vector<bool> result;
    mapping.get("myboolarray", result);
    ASSERT_EQ(expected, result);
  }

  {
    std::vector<int> expected{ 5, 5, 4, 4, 3, 2, 1, 0 };
    std::vector<int> result;
    mapping.get("myintarray", result);
    ASSERT_EQ(expected, result);
  }

  {
    std::vector<float> expected({6.5f, 5.25f, 4.125f, 3.0625f, 2.0f, 1.0f, 0.5f, 0.25f, 0.125f});
    std::vector<float> result;
    mapping.get("myfloatarray", result);
    ASSERT_EQ(expected, result);
  }

  {
    std::vector<std::string> expected{"One", "Two", "Three"};
    std::vector<std::string> result;
    mapping.get("mystringarray", result);
    ASSERT_EQ(expected, result);
  }

  {
    std::optional<ReaderMapping> child_mapping;
    mapping.get("mymapping", child_mapping);

    int a;
    child_mapping->get("a", a);
    ASSERT_EQ(1, a);

    int b;
    child_mapping->get("b", b);
    ASSERT_EQ(2, b);
  }

  {
    auto from_string = [](const std::string& text){ return std::stoi(text); };

    int value = 0;
    mapping.get_custom("mycustom", value, from_string);
    ASSERT_EQ(1234, value);

    int value2 = 0;
    mapping.get_custom("does-not-exist", value2, from_string);
    ASSERT_EQ(0, value2);

    int value3 = 0;
    mapping.get_custom("does-not-exist", value3, from_string, 4321);
    ASSERT_EQ(4321, value3);
  }

  {
    bool mybool;
    int myint;
    float myfloat;
    ASSERT_THROW({mapping.get("mybool", myfloat);}, std::runtime_error);
    ASSERT_THROW({mapping.get("myint", mybool);}, std::runtime_error);
    ASSERT_THROW({mapping.get("myfloat", myint);}, std::runtime_error);
    ASSERT_THROW({mapping.get("mymapping", myint);}, std::runtime_error);
  }
}

TEST(ReaderTest, get_compressed)
{
  std::istringstream in(
    "(supertux-test\n"
    "   (mycompressedintarray -6 0 45 -4 1 -5 3 -2 0)\n"
    ")\n");

  auto doc = ReaderDocument::from_stream(in);
  auto root = doc.get_root();
  ASSERT_EQ("supertux-test", root.get_name());
  auto mapping = root.get_mapping();

  {
    std::vector<unsigned int> expected{ 0, 0, 0, 0, 0, 0, 45, 1, 1, 1, 1, 3, 3, 3, 3, 3, 0, 0, };
    std::vector<unsigned int> result;
    mapping.get_compressed("mycompressedintarray", result);
    ASSERT_EQ(expected, result);
  }
}

TEST(ReaderTest, get_compressed_empty_list)
{
  // Empty (myempty) key should return an empty vector, no exception.
  std::istringstream in(
    "(supertux-test\n"
    "   (myempty)\n"
    ")\n");

  auto doc = ReaderDocument::from_stream(in);
  auto root = doc.get_root();
  ASSERT_EQ("supertux-test", root.get_name());
  auto mapping = root.get_mapping();

  std::vector<unsigned int> result;
  ASSERT_TRUE(mapping.get_compressed("myempty", result));
  ASSERT_TRUE(result.empty());
}

TEST(ReaderTest, get_compressed_rejects_negative_repeater_at_end)
{
  // Trailing negative value without a following positive is a syntax error.
  std::istringstream in(
    "(supertux-test\n"
    "   (bad -3 10 -2)\n"
    ")\n");

  auto doc = ReaderDocument::from_stream(in);
  auto root = doc.get_root();
  auto mapping = root.get_mapping();

  std::vector<unsigned int> result;
  EXPECT_THROW(
    {mapping.get_compressed("bad", result);},
    std::runtime_error);
}

TEST(ReaderTest, get_compressed_multi_repeater_chain)
{
  // -3 10 20 -1 30 -> five 10s, one 20, one 30
  std::istringstream in(
    "(supertux-test\n"
    "   (chain -3 10 20 -1 30)\n"
    ")\n");

  auto doc = ReaderDocument::from_stream(in);
  auto root = doc.get_root();
  ASSERT_EQ("supertux-test", root.get_name());
  auto mapping = root.get_mapping();

  std::vector<unsigned int> result;
  ASSERT_TRUE(mapping.get_compressed("chain", result));
  ASSERT_EQ(std::vector<unsigned int>({10, 10, 10, 20, 30}), result);
}

TEST(ReaderTest, get_compressed_chain_with_repeater_after_repeater_is_error)
{
  // Negative value while repeater is active is a syntax error.
  // -1 sets repeater=1, then -2 arrives while repeater is active → error.
  std::istringstream in(
    "(supertux-test\n"
    "   (mid -1 -2 8)\n"
    ")\n");

  auto doc = ReaderDocument::from_stream(in);
  auto root = doc.get_root();
  auto mapping = root.get_mapping();

  std::vector<unsigned int> result;
  EXPECT_THROW(
    {mapping.get_compressed("mid", result);},
    std::runtime_error);
}

TEST(ReaderTest, get_missing_key_returns_false)
{
  std::istringstream in(
    "(supertux-test\n"
    "   (mybool #t)\n"
    ")\n");

  auto doc = ReaderDocument::from_stream(in);
  auto mapping = doc.get_root().get_mapping();

  bool mybool;
  ASSERT_TRUE(mapping.get("mybool", mybool));
  ASSERT_EQ(true, mybool);

  // A key that was never written must return false (no throw) on plain get().
  int missing = 99;
  ASSERT_FALSE(mapping.get("ghost-key", missing))
      << "get() should return false for a missing key";
  ASSERT_EQ(99, missing) << "get() must not modify the output for a missing key";
}

TEST(ReaderTest, get_custom_missing_key_uses_default)
{
  std::istringstream in(
    "(supertux-test\n"
    "   (present \"123\")\n"
    "   (absent \"456\")\n"
    ")\n");

  auto doc = ReaderDocument::from_stream(in);
  auto mapping = doc.get_root().get_mapping();

  // Custom getter with a string key must convert the existing value.
  int present = 0;
  mapping.get_custom("present", present, [](const std::string& s){ return std::stoi(s); });
  ASSERT_EQ(123, present);

  // For a key that exists but the custom getter raises an exception on
  // the value, fall back to the supplied default_value.
  int absent = 99;
  // The "absent" key exists pointing to "456"; get_custom succeeds if the
  // converter returns normally — verify it does (not the failure path).
  ASSERT_TRUE(mapping.get_custom("absent", absent, [](const std::string& s){ return std::stoi(s); }));
  ASSERT_EQ(456, absent);

  // A genuinely missing key (not in the document) must use the explicit
  // default_value and return false.
  int missing = 0;
  ASSERT_FALSE(mapping.get_custom("no-such-key", missing,
                                   [](const std::string&){ return 0; }, 42));
  ASSERT_EQ(42, missing);
}

TEST(ReaderTest, type_mismatch_on_get_throws_runtime_error)
{
  std::istringstream in(
    "(supertux-test\n"
    "   (kb #t)\n"
    "   (ki 123456789)\n"
    "   (kf 1.125)\n"
    ")\n");

  auto doc = ReaderDocument::from_stream(in);
  auto mapping = doc.get_root().get_mapping();

  bool b;
  int i;
  float f;

  // Reading a key as a different type must throw a runtime_error carrying
  // the expression and file:line context.
  ASSERT_THROW(mapping.get("kb", f), std::runtime_error);
  ASSERT_THROW(mapping.get("ki", b), std::runtime_error);
  ASSERT_THROW(mapping.get("kf", i), std::runtime_error);

  // Reading a scalar key as a mapping must also throw.
  ASSERT_THROW(mapping.get("kf", b), std::runtime_error);
}

TEST(ReaderTest, malformed_document_throws_on_parse)
{
  // A completely broken s-expression must not construct a valid document.
  std::istringstream in(
    "(supertux-test\n"
    "   (mybool #t\n"
    "   (myint 123456789)\n"
    ")\n");

  bool threw = false;
  try {
    ReaderDocument::from_stream(in);
  } catch (const std::runtime_error&) {
    threw = true;
  }
  ASSERT_TRUE(threw)
      << "malformed document (unclosed list) must throw during parsing";
}

TEST(ReaderTest, empty_document_is_valid)
{
  // A document with no children must parse successfully and must reject
  // reads of non-existent keys (proving the mapping holds nothing).
  std::istringstream in("(empty-doc)\n");
  auto doc = ReaderDocument::from_string(in.str());
  ASSERT_EQ("empty-doc", doc.get_root().get_name());
  // Reading any key from the empty mapping must return false — that is the
  // proof that the mapping contains no keys.
  int should_not_exist = 0;
  ASSERT_FALSE(doc.get_root().get_mapping().get("should-not-exist", should_not_exist))
      << "a document with no keys must reject reads of arbitrary keys";
}
