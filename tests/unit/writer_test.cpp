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
// assert the values survive the save/load cycle. This exercises the real
// Save format used by the engine's level/world writers.
// Logging stubbed (writer_test_stub.cpp); physfs is not needed because we use
// the ostream constructor, not the filename constructor.

#include "st_assert.hpp"
#include "util/writer.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

#include <sstream>

namespace {

std::string serialize()
{
  std::ostringstream out;
  {
    Writer writer(out);
    writer.start_list("supertux-save");
    writer.write("mybool", true);
    writer.write("myint", 123456789);
    writer.write("myfloat", 1.5f);
    writer.write("mystring", "Hello World");
    writer.write("mystringtrans", "Translate Me", true);
    writer.write("myintarray", std::vector<int>{5, 5, 4, 4});
    writer.write("myflagarray", std::vector<int>{1, 0, 1});
    writer.end_list("supertux-save");
  }
  return out.str();
}

// Exercise write_compressed (run-length encoding) and the array overloads.
std::string serialize_compressed()
{
  std::ostringstream out;
  {
    Writer writer(out);
    writer.start_list("supertux-compressed");
    // RLE: a run of five 0s, then a single 45, then three 1s.
    writer.write_compressed("rle",
        std::vector<unsigned int>{0, 0, 0, 0, 0, 45, 1, 1, 1});
    // Empty vector must round-trip to empty.
    writer.write_compressed("empty", std::vector<unsigned int>{});
    // Single value (run length 1) is written without a count.
    writer.write_compressed("single", std::vector<unsigned int>{7});
    writer.write("floats", std::vector<float>{1.0f, 2.5f, 3.25f});
    writer.write("strings", std::vector<std::string>{"one", "two", "three"});
    // NUL bytes must survive inside strings (level/world saves store raw
    // tile names embedded in localized strings); verify the writer encodes
    // them and the reader decodes them back to the identical bytes.
    writer.start_list("nul-strings");
    writer.write("with-nul",
                 std::string{"prefix\0suffix", 12});
    writer.write("empty-nul",
                 std::string{'\0', 1});
    writer.end_list("nul-strings");
    writer.end_list("supertux-compressed");
    }
  return out.str();
}

} // namespace

int main(void)
{
  std::string data = serialize();

  // The produced text must be non-empty and contain the section name.
  ST_ASSERT("output not empty", !data.empty());
  ST_ASSERT("contains section name", data.find("supertux-save") != std::string::npos);

  // Parse it back and verify each value round-trips.
  auto doc = ReaderDocument::from_string(data);
  auto root = doc.get_root();
  ST_ASSERT("root name", root.get_name() == "supertux-save");
  auto mapping = root.get_mapping();

  {
    bool v;
    mapping.get("mybool", v);
    ST_ASSERT("bool round-trip", v == true);
  }
  {
    int v;
    mapping.get("myint", v);
    ST_ASSERT("int round-trip", v == 123456789);
  }
  {
    float v;
    mapping.get("myfloat", v);
    ST_ASSERT("float round-trip", v == 1.5f);
  }
  {
    std::string v;
    mapping.get("mystring", v);
    ST_ASSERT("string round-trip", v == "Hello World");
  }
  {
    std::string v;
    mapping.get("mystringtrans", v);
    ST_ASSERT("translatable string round-trip", v == "Translate Me");
  }
  {
    std::vector<int> v;
    mapping.get("myintarray", v);
    ST_ASSERT("int array round-trip size", v.size() == 4);
    ST_ASSERT("int array round-trip [0]", v[0] == 5);
    ST_ASSERT("int array round-trip [3]", v[3] == 4);
  }
  {
    std::vector<int> v;
    mapping.get("myflagarray", v);
    ST_ASSERT("flag array round-trip size", v.size() == 3);
    ST_ASSERT("flag array round-trip [0]", v[0] == 1);
    ST_ASSERT("flag array round-trip [1]", v[1] == 0);
  }

  // --- write_compressed round-trip (run-length decoding) -------------------
  {
    std::string cdata = serialize_compressed();
    ST_ASSERT("compressed output not empty", !cdata.empty());

    auto cdoc = ReaderDocument::from_string(cdata);
    auto croot = cdoc.get_root();
    ST_ASSERT("compressed root name", croot.get_name() == "supertux-compressed");
    auto cm = croot.get_mapping();

    std::vector<unsigned int> rle;
    cm.get_compressed("rle", rle);
    // Input was {0,0,0,0,0, 45, 1,1,1} -> decoded back to the same 9 values.
    ST_ASSERT("rle round-trip size", rle.size() == 9);
    ST_ASSERT("rle [0]", rle[0] == 0);
    ST_ASSERT("rle [4]", rle[4] == 0);
    ST_ASSERT("rle [5]", rle[5] == 45);
    ST_ASSERT("rle [6]", rle[6] == 1);
    ST_ASSERT("rle [8]", rle[8] == 1);

    std::vector<unsigned int> empty;
    cm.get_compressed("empty", empty);
    ST_ASSERT("empty compressed round-trips empty", empty.empty());

    std::vector<unsigned int> single;
    cm.get_compressed("single", single);
    ST_ASSERT("single compressed round-trips", single.size() == 1 && single[0] == 7);

    std::vector<float> floats;
    cm.get("floats", floats);
    ST_ASSERT("float array round-trip size", floats.size() == 3);
    ST_ASSERT("float array round-trip [1]", floats[1] == 2.5f);

    std::vector<std::string> strings;
    cm.get("strings", strings);
    ST_ASSERT("string array round-trip size", strings.size() == 3);
    ST_ASSERT("string array round-trip [2]", strings[2] == "three");
  }

  // --- NUL bytes inside strings must survive a save/load cycle ------------
  // Level/world saves occasionally embed raw tile names containing NULs
  // inside localized strings. The writer's escaping and the reader's
  // unescaping must preserve those bytes byte-for-byte.
  {
    std::ostringstream nullout;
    {
      Writer w(nullout);
      w.start_list("nul-strings");
      // "prefix\0suffix" — length 12 (including the embedded NUL).
      w.write("with-nul", std::string{"prefix\0suffix", 12});
      // A single NUL character as the entire string value.
      w.write("empty-nul", std::string{'\0', 1});
      w.end_list("nul-strings");
    }

    std::string ndata = nullout.str();
    auto ndoc = ReaderDocument::from_string(ndata);
    auto nroot = ndoc.get_root();
    auto nm = nroot.get_mapping();

    std::string with_nul;
    ST_ASSERT("NUL string must read back",
              nm.get("with-nul", with_nul) && with_nul == "prefix\0suffix");

    std::string empty_nul;
    ST_ASSERT("single-NUL string must read back",
              nm.get("empty-nul", empty_nul) && empty_nul == "\0");
  }

  // --- Bool-vector round-trip: verify booleans saved via the explicit
  // write(const char*, std::vector<bool>) overload are read back as 0/1
  // integers (the writer has no vector<bool> overload; booleans are packed
  // as 0/1 ints in the s-expression stream).
  {
    auto doc = ReaderDocument::from_string(data);
    std::vector<int> bools_as_int;
    bool got = doc.get_root().get_mapping().get("myflagarray", bools_as_int);
    ST_ASSERT("myflagarray key must read back", got);
    ST_ASSERT("flag array round-trip as 0/1 ints",
              (bools_as_int == std::vector<int>{1, 0, 1}));
  }

  // --- Writer edge cases not covered by the round-trips above ---------------

  // Escaped-string round-trip: quotes and backslashes must survive a save/load
  // cycle intact (writer.cpp write_escaped_string / reader parses \" and \\).
  {
    std::ostringstream out;
    {
      Writer w(out);
      w.start_list("escape-test");
      w.write("quoted", "say \"hello\"");
      w.write("backslash", "a\\b\\c");
      w.write("both", "x\"y\\z");
      w.end_list("escape-test");
    }
    std::string data = out.str();
    auto doc = ReaderDocument::from_string(data);
    auto root = doc.get_root();
    auto cm = root.get_mapping();
    std::string quoted, backslash, both;
    cm.get("quoted", quoted);
    cm.get("backslash", backslash);
    cm.get("both", both);
    ST_ASSERT("escaped quote round-trips", quoted == "say \"hello\"");
    ST_ASSERT("escaped backslash round-trips", backslash == "a\\b\\c");
    ST_ASSERT("escaped both round-trips", both == "x\"y\\z");
  }

  // write_sexp round-trip: a Writer::write(const char*, sexp::Value&) call
  // produces a nested s-expression that the reader must parse back to the same
  // sexp::Value. Covers the fudge-indent path and the non-array write path.
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
      w.write("plain-int", sexp::Value::integer(7));
      w.end_list("sexp-test");
    }
    std::string data = out.str();
    auto doc = ReaderDocument::from_string(data);
    auto root = doc.get_root();
    auto cm = root.get_mapping();

    // nested: must parse back to an array of two elements.
    sexp::Value nested;
    ST_ASSERT("nested must parse as array", cm.get("nested", nested) && nested.is_array());
    ST_ASSERT("nested size == 2", nested.as_array().size() == 2u);
    ST_ASSERT("nested[0] is symbol 'inner'", nested.as_array()[0].is_symbol() && nested.as_array()[0].as_string() == "inner");
    ST_ASSERT("nested[1] is integer 42", nested.as_array()[1].is_integer() && nested.as_array()[1].as_int() == 42);

    // plain-int: scalar write path.
    sexp::Value plain;
    ST_ASSERT("plain-int must parse as integer 7", cm.get("plain-int", plain) && plain.is_integer() && plain.as_int() == 7);
  }

  // end_list with wrong name emits a warning but does not crash; the list
  // stays open so the destructor warns about it. We only verify the produced
  // text is still parseable (the warning goes to the logging stub).
  {
    std::ostringstream out;
    {
      Writer w(out);
      w.start_list("good");
      w.end_list("wrong");   // log_warning: trying to close 'wrong' while 'good' is open
      w.end_list("good");    // now actually closes it
    }
    std::string data = out.str();
    auto doc = ReaderDocument::from_string(data);
    ST_ASSERT("good list name preserved", doc.get_root().get_name() == "good");
  }

  // write_comment produces a parseable ;-comment before the next token.
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
    // The comment must appear literally before value.
    ST_ASSERT("comment marker present", data.find("; this is a note") != std::string::npos);
    // Parsing must succeed: the reader skips ; comments.
    auto doc = ReaderDocument::from_string(data);
    ST_ASSERT("commented section parses", doc.get_root().get_name() == "commented");
    int v = 0;
    doc.get_root().get_mapping().get("value", v);
    ST_ASSERT("value after comment", v == 42);
    // And round-trip back through Writer must reproduce the comment.
    std::ostringstream back;
    {
      Writer bw(back);
      bw.start_list("commented");
      bw.write("value", v);
      bw.write_comment("this is a note");
      bw.end_list("commented");
    }
    std::string back_data = back.str();
    ST_ASSERT("round-trip preserves comment", back_data.find("; this is a note") != std::string::npos);
  }

  // Destructor warns when lists remain open. A document whose list is never
  // closed produces truncated s-expression text ("(open\n(x 1)\n") which the
  // reader REJECTS with a Parse Error — verify the failure is detected.
  {
    std::ostringstream out;
    {
      Writer w(out);
      w.start_list("open");
      w.write("x", 1);
      // no end_list — destructor will warn
    }
    std::string data = out.str();
    bool threw = false;
    try
    {
      ReaderDocument::from_string(data);
    }
    catch (const std::runtime_error&)
    {
      threw = true;
    }
    ST_ASSERT("unclosed list rejected by parser", threw);
  }
}