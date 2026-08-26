//  SuperTux
//  Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
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

// Reader coverage for the parts NOT exercised by reader_test.cpp:
//   - ReaderCollection::get_objects() (old "(section (obj ...) ...)" format)
//   - ReaderIterator::next()/get_key()/as_mapping() iteration loop
//   - default_value overloads on ReaderMapping::get(...)
//   - from_string + get_filename()/get_directory()
//   - uint32_t and UID get() overloads
//
// Engine logging/filesystem is stubbed by reader_test_stub.cpp (linked in
// alongside this file) to keep the test self-contained.

#include <gtest/gtest.h>

#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "util/reader_collection.hpp"
#include "util/reader_document.hpp"
#include "util/reader_iterator.hpp"
#include "util/reader_mapping.hpp"
#include "util/uid.hpp"

// Helper: parse a document whose root is (root-name ...body...) and return it.
static ReaderDocument
parse(const std::string& text, const std::string& filename = "<string>")
{
  return ReaderDocument::from_string(text, filename);
}

TEST(ReaderCollectionTest, get_objects)
{
  // Old-style collection: (sector (object "name" (x 1)) (object "name" (x 2)))
  std::string text =
    "(level\n"
    "  (objects\n"
    "    (sprite (name \"a\") (x 10))\n"
    "    (sprite (name \"b\") (x 20))\n"
    "    (sprite (name \"c\") (x 30))\n"
    "  )\n"
    ")\n";

  auto doc = parse(text);
  auto root = doc.get_root();
  ASSERT_EQ("level", root.get_name());

  auto mapping = root.get_mapping();
  std::optional<ReaderCollection> collection;
  ASSERT_TRUE(mapping.get("objects", collection));
  ASSERT_TRUE(collection.has_value());

  auto objects = collection->get_objects();
  ASSERT_EQ(3u, objects.size());

  // Each object carries its name and inner mapping.
  ASSERT_EQ("sprite", objects[0].get_name());
  ASSERT_EQ("sprite", objects[1].get_name());
  ASSERT_EQ("sprite", objects[2].get_name());

  int x0 = -1, x1 = -1, x2 = -1;
  objects[0].get_mapping().get("x", x0);
  objects[1].get_mapping().get("x", x1);
  objects[2].get_mapping().get("x", x2);
  ASSERT_EQ(10, x0);
  ASSERT_EQ(20, x1);
  ASSERT_EQ(30, x2);
}

TEST(ReaderCollectionTest, empty_collection)
{
  std::string text =
    "(level\n"
    "  (objects)\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();

  std::optional<ReaderCollection> collection;
  ASSERT_TRUE(mapping.get("objects", collection));
  ASSERT_TRUE(collection.has_value());
  ASSERT_EQ(0u, collection->get_objects().size());
}

TEST(ReaderIteratorTest, iterate_pairs)
{
  std::string text =
    "(supertux-test\n"
    "  (a 1)\n"
    "  (b 2)\n"
    "  (c 3)\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();
  auto it = mapping.get_iter();

  int seen = 0;
  std::vector<std::string> keys;
  while (it.next())
  {
    ASSERT_TRUE(it.is_pair());
    keys.push_back(it.get_key());
    if (it.get_key() == "a")
    {
      int v = 0;
      it.get(v);
      ASSERT_EQ(1, v);
      seen++;
    }
    else if (it.get_key() == "b")
    {
      int v = 0;
      it.get(v);
      ASSERT_EQ(2, v);
      seen++;
    }
    else if (it.get_key() == "c")
    {
      int v = 0;
      it.get(v);
      ASSERT_EQ(3, v);
      seen++;
    }
  }
  ASSERT_EQ(3, seen);
  ASSERT_EQ(std::vector<std::string>({"a", "b", "c"}), keys);
}

TEST(ReaderIteratorTest, iterate_string_items)
{
  std::string text =
    "(supertux-test\n"
    "  \"hello\" \"world\"\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();
  auto it = mapping.get_iter();

  std::vector<std::string> items;
  while (it.next())
  {
    ASSERT_TRUE(it.is_string());
    items.push_back(it.as_string_item());
  }
  ASSERT_EQ(std::vector<std::string>({"hello", "world"}), items);
}

TEST(ReaderIteratorTest, as_mapping)
{
  std::string text =
    "(supertux-test\n"
    "  (child (x 42))\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();
  auto it = mapping.get_iter();

  ASSERT_TRUE(it.next());
  ASSERT_EQ("child", it.get_key());
  auto child = it.as_mapping();
  int x = 0;
  child.get("x", x);
  ASSERT_EQ(42, x);
}

TEST(ReaderIteratorTest, as_mapping_on_string_item_is_error)
{
  // as_mapping() on a string item (not a pair) must throw / assert —
  // the ReaderIterator contract only supports mapping extraction on pair items.
  std::string text =
    "(supertux-test\n"
    "  \"just-a-string\"\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();
  auto it = mapping.get_iter();
  ASSERT_TRUE(it.next());
  ASSERT_TRUE(it.is_string());
  // as_mapping() on a non-pair item should fail — verify it throws
  // (the existing ReaderIterator implementation will call assert_is_array
  // which throws std::runtime_error via assert_is_array).
  EXPECT_THROW(it.as_mapping(), std::runtime_error);
}

TEST(ReaderMappingTest, default_value_overloads)
{
  // Keys present use the parsed value; missing keys fall back to default.
  std::string text =
    "(supertux-test\n"
    "  (present-int 7)\n"
    "  (present-str \"yes\")\n"
    "  (present-float 2.5)\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();

  int i = 0;
  ASSERT_TRUE(mapping.get("present-int", i, 99));
  ASSERT_EQ(7, i);

  int missing_i = 0;
  ASSERT_FALSE(mapping.get("absent-int", missing_i, 99));
  ASSERT_EQ(99, missing_i);

  std::string s;
  ASSERT_TRUE(mapping.get("present-str", s, std::optional<const char*>("fallback")));
  ASSERT_EQ("yes", s);

  std::string missing_s;
  ASSERT_FALSE(mapping.get("absent-str", missing_s, std::optional<const char*>("fallback")));
  ASSERT_EQ("fallback", missing_s);

  float f = 0.0f;
  ASSERT_TRUE(mapping.get("present-float", f, 1.0f));
  ASSERT_FLOAT_EQ(2.5f, f);

  float missing_f = 0.0f;
  ASSERT_FALSE(mapping.get("absent-float", missing_f, 1.0f));
  ASSERT_FLOAT_EQ(1.0f, missing_f);
}

TEST(ReaderMappingTest, uint32_and_uid_overloads)
{
  std::string text =
    "(supertux-test\n"
    "  (big 4294967295)\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();

  uint32_t big = 0;
  ASSERT_TRUE(mapping.get("big", big));
  ASSERT_EQ(4294967295u, big);

  // UID overload just needs a key that holds a string; absence returns false.
  UID uid;
  ASSERT_FALSE(mapping.get("no-such-uid", uid));
}

TEST(ReaderMappingTest, translations_disabled_returns_raw_string)
{
  // The editor parses levels with s_translations_enabled = false so that
  // (_ "text") entries come through verbatim instead of via gettext().
  std::string text =
    "(supertux-test\n"
    "  (name (_ \"Hollow Hills\"))\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();

  std::string name;
  ASSERT_TRUE(mapping.get("name", name));
  ASSERT_EQ("Hollow Hills", name); // translated path (stub _ is identity)

  ReaderMapping::s_translations_enabled = false;
  std::string raw;
  ASSERT_TRUE(mapping.get("name", raw));
  ASSERT_EQ("Hollow Hills", raw);
  ReaderMapping::s_translations_enabled = true; // restore global state
}

TEST(ReaderObjectTest, get_collection_and_get_mapping_views)
{
  // Same root sexp viewed as mapping vs collection: get_mapping() reads
  // key/value pairs, get_collection()/get_objects() enumerates children.
  std::string text =
    "(sector\n"
    "  (tilemap (zpos -100))\n"
    "  (background (color 1 1 1))\n"
    ")\n";

  auto doc = parse(text);
  auto root = doc.get_root();

  ASSERT_EQ("sector", root.get_name());

  // Each child object is itself readable as a mapping of its own pairs.
  auto objects = root.get_collection().get_objects();
  ASSERT_EQ(2u, objects.size());
  ASSERT_EQ("tilemap", objects[0].get_name());
  ASSERT_EQ("background", objects[1].get_name());

  int zpos = 0;
  objects[0].get_mapping().get("zpos", zpos);
  ASSERT_EQ(-100, zpos);
}

TEST(ReaderMappingTest, get_custom_with_translatable_string)
{
  std::string text =
    "(supertux-test\n"
    "  (direction (_ \"left\"))\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();

  auto from_string = [](const std::string& s) { return s; };
  std::string dir;
  ASSERT_TRUE(mapping.get_custom("direction", dir, from_string));
  ASSERT_EQ("left", dir);
}

TEST(ReaderDocumentTest, from_string_filename_and_directory)
{
  std::string text = "(supertux-test (a 1))\n";
  auto doc = parse(text, "/path/to/levels/world1.stl");

  ASSERT_EQ("supertux-test", doc.get_root().get_name());
  ASSERT_EQ("/path/to/levels/world1.stl", doc.get_filename());
  ASSERT_EQ("/path/to/levels", doc.get_directory());
}

TEST(ReaderDocumentTest, from_stream_equivalent)
{
  std::string text = "(supertux-test (a 1))\n";
  std::istringstream in(text);
  auto doc = ReaderDocument::from_stream(in, "<stream>");

  ASSERT_EQ("supertux-test", doc.get_root().get_name());
  int a = 0;
  doc.get_root().get_mapping().get("a", a);
  ASSERT_EQ(1, a);
}

TEST(ReaderDocumentTest, get_filename_and_directory_on_relative_path)
{
  // Relative path without directory component: get_directory() returns empty.
  std::string text = "(supertux-test (a 1))\n";
  auto doc = parse(text, "level.stl");

  ASSERT_EQ("level.stl", doc.get_filename());
  ASSERT_TRUE(doc.get_directory().empty());
}

TEST(ReaderDocumentTest, get_filename_and_directory_on_deep_path)
{
  std::string text = "(supertux-test (a 1))\n";
  auto doc = parse(text, "/a/b/c/level.stl");

  ASSERT_EQ("/a/b/c/level.stl", doc.get_filename());
  ASSERT_EQ("/a/b/c", doc.get_directory());
}

TEST(ReaderMappingTest, get_sexp_value)
{
  // get(const char*, sexp::Value&) exposes the raw value sexp for a key.
  std::string text =
    "(supertux-test\n"
    "  (integer 42)\n"
    "  (boolean #t)\n"
    "  (string \"hello\")\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();

  sexp::Value v;
  ASSERT_TRUE(mapping.get("integer", v));
  ASSERT_TRUE(v.is_integer());
  ASSERT_EQ(42, v.as_int());

  ASSERT_TRUE(mapping.get("boolean", v));
  ASSERT_TRUE(v.is_boolean());
  ASSERT_TRUE(v.as_bool());

  ASSERT_TRUE(mapping.get("string", v));
  ASSERT_TRUE(v.is_string());
  ASSERT_EQ("hello", v.as_string());

  // Missing key → returns false, value untouched.
  sexp::Value missing;
  ASSERT_FALSE(mapping.get("no-such-key", missing));
}

TEST(ReaderMappingTest, get_optional_mapping_not_found)
{
  // std::optional<ReaderMapping> overload returns false when the key is absent.
  std::string text =
    "(supertux-test\n"
    "  (present (x 1))\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();

  std::optional<ReaderMapping> found;
  ASSERT_TRUE(mapping.get("present", found));
  ASSERT_TRUE(found.has_value());
  int x = 0;
  found->get("x", x);
  ASSERT_EQ(1, x);

  std::optional<ReaderMapping> missing;
  ASSERT_FALSE(mapping.get("no-such-mapping", missing));
  ASSERT_FALSE(missing.has_value());
}

TEST(ReaderMappingTest, get_optional_collection_not_found)
{
  // std::optional<ReaderCollection> overload returns false when the key is absent.
  // Use a FRESH optional for the not-found case (reusing a populated one
  // would still report has_value() == true since get() does not reset on miss).
  std::string text =
    "(supertux-test\n"
    "  (things (item 1) (item 2))\n"
    "  (other (item 3))\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();

  std::optional<ReaderCollection> coll;
  ASSERT_TRUE(mapping.get("things", coll));
  ASSERT_TRUE(coll.has_value());
  ASSERT_EQ(2u, coll->get_objects().size());

  std::optional<ReaderCollection> missing;
  ASSERT_FALSE(mapping.get("no-such-collection", missing));
  ASSERT_FALSE(missing.has_value());
}

// Positive UID get() case: the existing uint32_and_uid_overloads test only
// covers the absent-key path. Real semantics: (key 42) parses as UID(42),
// and a zero value is rejected as an invalid UID.
TEST(ReaderMappingTest, uid_get_positive_and_zero_value)
{
  std::string text =
    "(supertux-test\n"
    "  (myuid 42)\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();

  UID uid;
  ASSERT_TRUE(mapping.get("myuid", uid));
  ASSERT_TRUE(static_cast<bool>(uid));
  ASSERT_EQ(42u, uid.get_value());

  // Zero encodes "no UID"; the reader accepts the parse but the object is falsy.
  auto doc2 = ReaderDocument::from_string("(supertux-test\n  (zuid 0)\n)\n");
  UID zuid;
  if (doc2.get_root().get_mapping().get("zuid", zuid))
  {
    ASSERT_FALSE(static_cast<bool>(zuid));
  }
}


// --- Back-door API coverage ----------------------------------------------

TEST(ReaderObjectTest, get_name_on_non_object_throws)
{
  // get_name() requires an array whose head is a symbol. A bare string or
  // integer item is NOT an object — the reader asserts with a runtime_error.
  auto doc = parse("(supertux-test\n  \"just-a-string\"\n)\n");
  auto objects = doc.get_root().get_collection().get_objects();
  ASSERT_EQ(1u, objects.size());
  EXPECT_THROW(objects[0].get_name(), std::runtime_error);

  auto idoc = parse("(supertux-test\n  42\n)\n");
  auto iobjects = idoc.get_root().get_collection().get_objects();
  ASSERT_EQ(1u, iobjects.size());
  EXPECT_THROW(iobjects[0].get_name(), std::runtime_error);
}

TEST(ReaderObjectTest, get_mapping_on_symbol_head_reads_pairs)
{
  // get_mapping() works on any object regardless of shape; reading a key
  // that does not exist simply fails (returns false), no throw.
  std::string text =
    "(supertux-test\n"
    "  (tile (zpos -100))\n"
    ")\n";
  auto doc = parse(text);
  auto objects = doc.get_root().get_collection().get_objects();
  ASSERT_EQ("tile", objects[0].get_name());
  int zpos = 0;
  ASSERT_TRUE(objects[0].get_mapping().get("zpos", zpos));
  ASSERT_EQ(-100, zpos);
  int absent = -5;
  ASSERT_FALSE(objects[0].get_mapping().get("nope", absent));
  ASSERT_EQ(-5, absent);  // untouched on miss
}

TEST(ReaderMappingTest, get_sexp_value_overload)
{
  // get(const char*, sexp::Value&) returns the raw sexp value for a key.
  // This is the back-door for getting untyped sexp values out of the reader.
  std::string text =
    "(supertux-test\n"
    "  (integer 42)\n"
    "  (boolean #t)\n"
    "  (string \"hello\")\n"
    "  (nested (a 1))\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();

  sexp::Value v;
  ASSERT_TRUE(mapping.get("integer", v));
  ASSERT_TRUE(v.is_integer());
  ASSERT_EQ(42, v.as_int());

  ASSERT_TRUE(mapping.get("boolean", v));
  ASSERT_TRUE(v.is_boolean());
  ASSERT_TRUE(v.as_bool());

  ASSERT_TRUE(mapping.get("string", v));
  ASSERT_TRUE(v.is_string());
  ASSERT_EQ("hello", v.as_string());

  // Nested mapping comes back as a sexp array.
  ASSERT_TRUE(mapping.get("nested", v));
  ASSERT_TRUE(v.is_array());
  ASSERT_EQ(2u, v.as_array().size());
  ASSERT_TRUE(v.as_array()[0].is_symbol());
  ASSERT_EQ("a", v.as_array()[0].as_string());
  ASSERT_EQ(1, v.as_array()[1].as_int());

  // Missing key returns false, v is untouched.
  sexp::Value missing;
  ASSERT_FALSE(mapping.get("no-such-key", missing));
}

TEST(ReaderMappingTest, get_optional_reader_mapping_not_found)
{
  // get(key, optional<ReaderMapping>&) returns false when the key is absent.
  std::string text =
    "(supertux-test\n"
    "  (present (x 1))\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();

  std::optional<ReaderMapping> found;
  ASSERT_TRUE(mapping.get("present", found));
  ASSERT_TRUE(found.has_value());
  int x = 0;
  found->get("x", x);
  ASSERT_EQ(1, x);

  std::optional<ReaderMapping> missing;
  ASSERT_FALSE(mapping.get("no-such-key", missing));
  ASSERT_FALSE(missing.has_value());
}

TEST(ReaderMappingTest, get_optional_reader_collection_not_found)
{
  // get(key, optional<ReaderCollection>&) returns false when the key is absent.
  std::string text =
    "(supertux-test\n"
    "  (things (item 1) (item 2))\n"
    ")\n";

  auto doc = parse(text);
  auto mapping = doc.get_root().get_mapping();

  std::optional<ReaderCollection> found;
  ASSERT_TRUE(mapping.get("things", found));
  ASSERT_TRUE(found.has_value());
  ASSERT_EQ(2u, found->get_objects().size());

  std::optional<ReaderCollection> missing;
  ASSERT_FALSE(mapping.get("no-such-key", missing));
  ASSERT_FALSE(missing.has_value());
}

/* EOF */
