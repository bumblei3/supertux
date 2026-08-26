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

// End-to-end coverage for supertux/tile_set_parser.cpp: parse tileset
// config content (tiles, tilegroups with range filtering and offsets)
// into a real TileSet. Uses real temp files like AutotileParserTest;
// same stub/link strategy (stdio-backed PHYSFS shims).

#include <gtest/gtest.h>

#include "supertux/globals.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/tile_set.hpp"
#include "supertux/tile_set_parser.hpp"
#include "supertux/tile.hpp"

#include <cstdio>
#include <fstream>
#include <string>

namespace {

Config test_config;

std::string write_temp(const std::string& content)
{
  static int counter = 0;
  std::string path = "/tmp/st_tileset_" + std::to_string(getpid()) + "_" + std::to_string(counter++) + ".cfg";
  std::ofstream out(path, std::ios::trunc);
  out << content;
  out.close();
  if (!out.good())
    throw std::runtime_error("failed to write temp file: " + path);
  return path;
}

} // namespace

class TileSetParserTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    g_config = &test_config;
    test_config.developer_mode = false;
  }
  void TearDown() override { g_config = nullptr; }
};

TEST_F(TileSetParserTest, wrong_root_element_throws)
{
  const std::string path = write_temp("(something-else\n)\n");

  TileSet tileset(path);
  TileSetParser parser(tileset, path);
  EXPECT_THROW(parser.parse(), std::runtime_error);
}

TEST_F(TileSetParserTest, missing_file_throws)
{
  TileSet tileset("/tmp/does-not-exist-st.cfg");
  TileSetParser parser(tileset, "/tmp/does-not-exist-st.cfg");
  EXPECT_THROW(parser.parse(), std::runtime_error);
}

TEST_F(TileSetParserTest, parses_tile_and_registers_it)
{
  const std::string path = write_temp(
    "(supertux-tiles\n"
    "  (tile\n"
    "    (id 7)\n"
    "    (solid #t)\n"
    "  )\n"
    ")\n");

  TileSet tileset(path);
  TileSetParser parser(tileset, path);
  parser.parse();

  // Real semantics: add_tile grows m_tiles so max id reflects the highest id+1.
  EXPECT_GE(tileset.get_max_tileid(), 8u);
  const Tile& tile = tileset.get(7u);
  EXPECT_TRUE(tile.is_solid());
}

TEST_F(TileSetParserTest, tilegroup_range_filter_zeroes_out_of_range_tiles)
{
  const std::string path = write_temp(
    "(supertux-tiles\n"
    "  (tile\n"
    "    (id 50)\n"
    "  )\n"
    "  (tilegroup\n"
    "    (name \"grp\")\n"
    "    (offset 0)\n"
    "    (tiles 10 50 90)\n"   // 10 and 90 outside [20..80] -> zeroed
    "  )\n"
    ")\n");

  TileSet tileset(path);
  TileSetParser parser(tileset, path, 20, 80);
  parser.parse();

  ASSERT_FALSE(tileset.get_tilegroups().empty());
  const Tilegroup& grp = tileset.get_tilegroups()[0];
  EXPECT_EQ("grp", grp.name);
  ASSERT_EQ(3u, grp.tiles.size());
  EXPECT_EQ(0, grp.tiles[0]);   // below start -> zeroed
  EXPECT_EQ(50, grp.tiles[1]);  // in range -> kept
  EXPECT_EQ(0, grp.tiles[2]);   // above end -> zeroed
}

TEST_F(TileSetParserTest, tilegroup_offset_applied_to_valid_tiles)
{
  const std::string path = write_temp(
    "(supertux-tiles\n"
    "  (tilegroup\n"
    "    (name \"shifted\")\n"
    "    (offset 100)\n"
    "    (tiles 5)\n"
    "  )\n"
    ")\n");

  TileSet tileset(path);
  // Parser offset 10 + group offset 100 => 5 + 110 = 115.
  TileSetParser parser(tileset, path, 0, 0, 10);
  parser.parse();

  ASSERT_FALSE(tileset.get_tilegroups().empty());
  EXPECT_EQ(115, tileset.get_tilegroups()[0].tiles[0]);
}
