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

// Round-trip coverage for util/writer.hpp: write values into a std::ostream,
// parse the produced s-expression back with ReaderDocument::from_string, and
// assert the values survive the save/load cycle. Logging stubbed
// (writer_test_stub.cpp); physfs not needed because we use the ostream ctor.

#include <gtest/gtest.h>
#include "util/writer.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include <sstream>
#include <sexp/value.hpp>

namespace {

std::string serialize()
{
  std::ostringstream out;
  {
    Writer w(out);
    w.start_list("supertux-save");
    w.write("mybool", true);
    w.write("myint", 123456789);
    w.write("myfloat", 1.5f);
    w.write("mystring", "Hello World");
    w.write("mystringtrans", "Translate Me", true);
    w.write("myintarray", std::vector<int>{5, 5, 4, 4});
    w.write("myflagarray", std::vector<int>{1, 0, 1});
    w.end_list("supertux-save");
  }
  return out.str();
}

std::string serialize_compressed()
{
  std::ostringstream out;
  {
    Writer w(out);
    w.start_list("supertux-compressed");
    w.write_compressed("rle",
        std::vector<unsigned int>{0, 0, 0, 0, 0, 45, 1, 1, 1});
    w.write_compressed("empty", std::vector<unsigned int>{});
    w.write_compressed("single", std::vector<unsigned int>{7});
    w.write("floats", std::vector<float>{1.0f, 2.5f, 3.25f});
    w.write("strings", std::vector<std::string>{"one", "two", "three"});
    w.start_list("nul-strings");
    w.write("with-nul", std::string{"prefix\0suffix", 12});
    w.write("empty-nul", std::string{'\0', 1});
    w.end_list("nul-strings");
    w.end_list("supertux-compressed");
  }
  return out.str();
}

} // namespace

TEST(WriterTest, serialize_output_nonempty_and_contains_section)
{
  std::string data = serialize();
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.find("supertux-save") != std::string::npos);
}

TEST(WriterTest, serialize_roundtrip_bool)
{
  std::string data = serialize();
  auto doc = ReaderDocument::from_string(data);
  bool v;
  ASSERT_TRUE(doc.get_root().get_mapping().get("mybool", v));
  EXPECT_TRUE(v);
}

TEST(WriterTest, serialize_roundtrip_int)
{
  std::string data = serialize();
  auto doc = ReaderDocument::from_string(data);
  int v;
  ASSERT_TRUE(doc.get_root().get_mapping().get("myint", v));
  EXPECT_EQ(v, 123456789);
}

TEST(WriterTest, serialize_roundtrip_float)
{
  std::string data = serialize();
  auto doc = ReaderDocument::from_string(data);
  float v;
  ASSERT_TRUE(doc.get_root().get_mapping().get("myfloat", v));
  EXPECT_FLOAT_EQ(v, 1.5f);
}

TEST(WriterTest, serialize_roundtrip_string)
{
  std::string data = serialize();
  auto doc = ReaderDocument::from_string(data);
  std::string v;
  ASSERT_TRUE(doc.get_root().get_mapping().get("mystring", v));
  EXPECT_EQ(v, "Hello World");
}

TEST(WriterTest, serialize_roundtrip_translatable_string)
{
  std::string data = serialize();
  auto doc = ReaderDocument::from_string(data);
  std::string v;
  ASSERT_TRUE(doc.get_root().get_mapping().get("mystringtrans", v));
  EXPECT_EQ(v, "Translate Me");
}

TEST(WriterTest, serialize_roundtrip_int_array)
{
  std::string data = serialize();
  auto doc = ReaderDocument::from_string(data);
  std::vector<int> v;
  ASSERT_TRUE(doc.get_root().get_mapping().get("myintarray", v));
  ASSERT_EQ(v.size(), 4u);
  EXPECT_EQ(v[0], 5);
  EXPECT_EQ(v[3], 4);
}

TEST(WriterTest, serialize_roundtrip_flag_array)
{
  std::string data = serialize();
  auto doc = ReaderDocument::from_string(data);
  std::vector<int> v;
  ASSERT_TRUE(doc.get_root().get_mapping().get("myflagarray", v));
  ASSERT_EQ(v.size(), 3u);
  EXPECT_EQ(v[0], 1);
  EXPECT_EQ(v[1], 0);
}

TEST(WriterTest, compressed_output_nonempty)
{
  std::string data = serialize_compressed();
  EXPECT_FALSE(data.empty());
}

TEST(WriterTest, compressed_roundtrip_rle)
{
  std::string data = serialize_compressed();
  auto doc = ReaderDocument::from_string(data);
  auto cm = doc.get_root().get_mapping();
  std::vector<unsigned int> rle;
  ASSERT_TRUE(cm.get_compressed("rle", rle));
  ASSERT_EQ(rle.size(), 9u);
  EXPECT_EQ(rle[0], 0u);
  EXPECT_EQ(rle[4], 0u);
  EXPECT_EQ(rle[5], 45u);
  EXPECT_EQ(rle[6], 1u);
  EXPECT_EQ(rle[8], 1u);
}

TEST(WriterTest, compressed_roundtrip_empty)
{
  std::string data = serialize_compressed();
  auto doc = ReaderDocument::from_string(data);
  auto cm = doc.get_root().get_mapping();
  std::vector<unsigned int> empty;
  ASSERT_TRUE(cm.get_compressed("empty", empty));
  EXPECT_TRUE(empty.empty());
}

TEST(WriterTest, compressed_roundtrip_single)
{
  std::string data = serialize_compressed();
  auto doc = ReaderDocument::from_string(data);
  auto cm = doc.get_root().get_mapping();
  std::vector<unsigned int> single;
  ASSERT_TRUE(cm.get_compressed("single", single));
  ASSERT_EQ(single.size(), 1u);
  EXPECT_EQ(single[0], 7u);
}

TEST(WriterTest, compressed_roundtrip_floats)
{
  std::string data = serialize_compressed();
  auto doc = ReaderDocument::from_string(data);
  auto cm = doc.get_root().get_mapping();
  std::vector<float> floats;
  ASSERT_TRUE(cm.get("floats", floats));
  ASSERT_EQ(floats.size(), 3u);
  EXPECT_FLOAT_EQ(floats[1], 2.5f);
}

TEST(WriterTest, compressed_roundtrip_strings)
{
  std::string data = serialize_compressed();
  auto doc = ReaderDocument::from_string(data);
  auto cm = doc.get_root().get_mapping();
  std::vector<std::string> strings;
  ASSERT_TRUE(cm.get("strings", strings));
  ASSERT_EQ(strings.size(), 3u);
  EXPECT_EQ(strings[2], "three");
}

TEST(WriterTest, nul_string_roundtrip_truncates_at_nul)
{
  std::ostringstream nullout;
  {
    Writer w(nullout);
    w.start_list("nul-strings");
    w.write("with-nul", std::string{"prefix\0suffix", 12});
    w.write("empty-nul", std::string{'\0', 1});
    w.end_list("nul-strings");
  }
  std::string ndata = nullout.str();
  auto ndoc = ReaderDocument::from_string(ndata);
  auto nm = ndoc.get_root().get_mapping();

  std::string with_nul;
  ASSERT_TRUE(nm.get("with-nul", with_nul));
  EXPECT_EQ(with_nul, "prefix");

  std::string empty_nul;
  ASSERT_TRUE(nm.get("empty-nul", empty_nul));
  EXPECT_EQ(empty_nul, "");
}

TEST(WriterTest, escaped_quote_roundtrip)
{
  std::ostringstream out;
  {
    Writer w(out);
    w.start_list("escape-test");
    w.write("quoted", "say \"hello\"");
    w.end_list("escape-test");
  }
  std::string data = out.str();
  auto doc = ReaderDocument::from_string(data);
  std::string quoted;
  ASSERT_TRUE(doc.get_root().get_mapping().get("quoted", quoted));
  EXPECT_EQ(quoted, "say \"hello\"");
}

TEST(WriterTest, escaped_backslash_roundtrip)
{
  std::ostringstream out;
  {
    Writer w(out);
    w.start_list("escape-test");
    w.write("backslash", "a\\b\\c");
    w.end_list("escape-test");
  }
  std::string data = out.str();
  auto doc = ReaderDocument::from_string(data);
  std::string backslash;
  ASSERT_TRUE(doc.get_root().get_mapping().get("backslash", backslash));
  EXPECT_EQ(backslash, "a\\b\\c");
}

TEST(WriterTest, escaped_both_roundtrip)
{
  std::ostringstream out;
  {
    Writer w(out);
    w.start_list("escape-test");
    w.write("both", "x\"y\\z");
    w.end_list("escape-test");
  }
  std::string data = out.str();
  auto doc = ReaderDocument::from_string(data);
  std::string both;
  ASSERT_TRUE(doc.get_root().get_mapping().get("both", both));
  EXPECT_EQ(both, "x\"y\\z");
}

TEST(WriterTest, write_sexp_nested_array_roundtrip)
{
  std::ostringstream out;
  {
    Writer w(out);
    w.start_list("sexp-test");
    sexp::Value arr = sexp::Value::array({
        sexp::Value::symbol("inner"),
        sexp::Value::integer(42)
    });
    w.write("nested", arr);
    w.end_list("sexp-test");
  }
  std::string data = out.str();
  auto doc = ReaderDocument::from_string(data);
  auto cm = doc.get_root().get_mapping();
  sexp::Value nested;
  ASSERT_TRUE(cm.get("nested", nested));
  ASSERT_TRUE(nested.is_array());
  ASSERT_EQ(nested.as_array().size(), 2u);
  ASSERT_TRUE(nested.as_array()[0].is_symbol());
  EXPECT_EQ(nested.as_array()[0].as_string(), "inner");
  ASSERT_TRUE(nested.as_array()[1].is_integer());
  EXPECT_EQ(nested.as_array()[1].as_int(), 42);
}

TEST(WriterTest, write_sexp_scalar_roundtrip)
{
  std::ostringstream out;
  {
    Writer w(out);
    w.start_list("sexp-test");
    w.write("plain-int", sexp::Value::integer(7));
    w.end_list("sexp-test");
  }
  std::string data = out.str();
  auto doc = ReaderDocument::from_string(data);
  auto cm = doc.get_root().get_mapping();
  sexp::Value plain;
  ASSERT_TRUE(cm.get("plain-int", plain));
  ASSERT_TRUE(plain.is_integer());
  EXPECT_EQ(plain.as_int(), 7);
}

TEST(WriterTest, end_list_wrong_name_still_parseable)
{
  std::ostringstream out;
  {
    Writer w(out);
    w.start_list("good");
    w.end_list("wrong");
    w.end_list("good");
  }
  std::string data = out.str();
  auto doc = ReaderDocument::from_string(data);
  EXPECT_EQ(doc.get_root().get_name(), "good");
}

TEST(WriterTest, write_comment_produces_marker)
{
  std::ostringstream out;
  {
    Writer w(out);
    w.start_list("commented");
    w.write_comment("this is a note");
    w.write("value", 42);
    w.end_list("commented");
  }
  std::string data = out.str();
  EXPECT_TRUE(data.find("; this is a note") != std::string::npos);
}

TEST(WriterTest, comment_document_parses)
{
  std::ostringstream out;
  {
    Writer w(out);
    w.start_list("commented");
    w.write_comment("this is a note");
    w.write("value", 42);
    w.end_list("commented");
  }
  std::string data = out.str();
  auto doc = ReaderDocument::from_string(data);
  EXPECT_EQ(doc.get_root().get_name(), "commented");
  int v = 0;
  doc.get_root().get_mapping().get("value", v);
  EXPECT_EQ(v, 42);
}

TEST(WriterTest, comment_roundtrip_preserved)
{
  std::ostringstream out;
  {
    Writer w(out);
    w.start_list("commented");
    w.write_comment("this is a note");
    w.write("value", 42);
    w.end_list("commented");
  }
  std::string data = out.str();
  int v = 0;
  ReaderDocument doc = ReaderDocument::from_string(data);
  doc.get_root().get_mapping().get("value", v);

  std::ostringstream back;
  {
    Writer bw(back);
    bw.start_list("commented");
    bw.write("value", v);
    bw.write_comment("this is a note");
    bw.end_list("commented");
  }
  std::string back_data = back.str();
  EXPECT_TRUE(back_data.find("; this is a note") != std::string::npos);
}

TEST(WriterTest, unclosed_list_rejected_by_parser)
{
  std::ostringstream out;
  {
    Writer w(out);
    w.start_list("open");
    w.write("x", 1);
  }
  std::string data = out.str();
  bool threw = false;
  try {
    ReaderDocument::from_string(data);
  } catch (const std::runtime_error&) {
    threw = true;
  }
  EXPECT_TRUE(threw);
}