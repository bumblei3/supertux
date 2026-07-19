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
}

/* EOF */
