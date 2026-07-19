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
}

/* EOF */
