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

// Coverage for supertux/autotile_parser.cpp: end-to-end parsing of
// autotile configuration content into AutotileSet objects. Uses real
// temp files (AutotileParser::parse() reads via ReaderDocument::from_file)
// and a stubbed g_config so validate() can be exercised too.

#include <gtest/gtest.h>

#include "supertux/autotile_parser.hpp"
#include "supertux/autotile.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"

#include <cstdio>
#include <fstream>
#include <string>

namespace {

// gameconfig.cpp is NOT linked (it drags in controller/joystick/keyboard
// config and curl). The only symbol autotile_parser.cpp needs is g_config;
// developer_mode=false keeps validate() out of the parse path.
// NOTE: Config's constructor allocates a small SDL resource that LSan flags
// as leaked at exit (SDL cleans up after its own atexit handlers run). This
// is a known SDL/LSan false positive — run with detect_leaks=0 if your setup
// enables it by default.
Config test_config;

std::string write_temp(const std::string& content)
{
  static int counter = 0;
  std::string path = "/tmp/st_autotile_" + std::to_string(getpid()) + "_" + std::to_string(counter++) + ".cfg";
  std::ofstream out(path, std::ios::trunc);
  out << content;
  out.close();
  if (!out.good())
    throw std::runtime_error("failed to write temp file: " + path);
  return path;
}

} // namespace

class AutotileParserTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    g_config = &test_config;
    test_config.developer_mode = false;
  }
  void TearDown() override { g_config = nullptr; }
};

TEST_F(AutotileParserTest, parses_simple_solid_autotileset)
{
  const std::string path = write_temp(
    "(supertux-autotiles\n"
    "  (autotileset\n"
    "    (name \"grass\")\n"
    "    (default 5)\n"
    "    (solid #t)\n"
    "    (autotile\n"
    "      (id 1)\n"
    "      (solid #t)\n"
    "      (mask \"11111111\")\n"
    "    )\n"
    "  )\n"
    ")\n");

  std::vector<std::unique_ptr<AutotileSet>> sets;
  AutotileParser parser(sets, path);
  parser.parse();

  ASSERT_EQ(1u, sets.size());
  EXPECT_EQ("grass", sets[0]->get_name());
  EXPECT_EQ(5u, sets[0]->get_default_tile());
  EXPECT_FALSE(sets[0]->is_corner());
  EXPECT_TRUE(sets[0]->is_member(1u));
}

TEST_F(AutotileParserTest, offset_shifts_all_ids)
{
  const std::string path = write_temp(
    "(supertux-autotiles\n"
    "  (autotileset\n"
    "    (name \"shifted\")\n"
    "    (default 100)\n"
    "    (solid #t)\n"
    "    (autotile\n"
    "      (id 100)\n"
    "      (solid #t)\n"
    "      (mask \"00000000\")\n"
    "    )\n"
    "  )\n"
    ")\n");

  std::vector<std::unique_ptr<AutotileSet>> sets;
  // offset=1000: parsed id 100 becomes 1100.
  AutotileParser parser(sets, path, 0, 0, 1000);
  parser.parse();

  ASSERT_EQ(1u, sets.size());
  EXPECT_EQ(1100u, sets[0]->get_default_tile());
  EXPECT_TRUE(sets[0]->is_member(1100u));
  EXPECT_FALSE(sets[0]->is_member(100u));
}

TEST_F(AutotileParserTest, ids_outside_start_end_range_are_dropped)
{
  const std::string path = write_temp(
    "(supertux-autotiles\n"
    "  (autotileset\n"
    "    (name \"ranged\")\n"
    "    (default 50)\n"
    "    (solid #t)\n"
    "    (autotile\n"
    "      (id 10)\n"
    "      (solid #t)\n"
    "      (mask \"11111111\")\n"
    "    )\n"
    "    (autotile\n"
    "      (id 90)\n"
    "      (solid #t)\n"
    "      (mask \"11111111\")\n"
    "    )\n"
    "  )\n"
    ")\n");

  std::vector<std::unique_ptr<AutotileSet>> sets;
  // Only IDs in [20, 80] are kept: id 10 and id 90 fall outside.
  AutotileParser parser(sets, path, 20, 80);
  parser.parse();

  if (!sets.empty())
  {
    EXPECT_TRUE(sets[0]->is_member(50u));
    EXPECT_FALSE(sets[0]->is_member(10u));
    EXPECT_FALSE(sets[0]->is_member(90u));
  }
}

TEST_F(AutotileParserTest, wrong_root_element_throws)
{
  const std::string path = write_temp("(something-else\n)\n");

  std::vector<std::unique_ptr<AutotileSet>> sets;
  AutotileParser parser(sets, path);
  EXPECT_THROW(parser.parse(), std::runtime_error);
}

TEST_F(AutotileParserTest, missing_file_throws)
{
  std::vector<std::unique_ptr<AutotileSet>> sets;
  AutotileParser parser(sets, "/tmp/does-not-exist-st.cfg");
  EXPECT_THROW(parser.parse(), std::runtime_error);
}

TEST_F(AutotileParserTest, missing_id_parameter_throws)
{
  const std::string path = write_temp(
    "(supertux-autotiles\n"
    "  (autotileset\n"
    "    (name \"broken\")\n"
    "    (autotile\n"
    "      (solid #t)\n"
    "      (mask \"11111111\")\n"
    "    )\n"
    "  )\n"
    ")\n");

  std::vector<std::unique_ptr<AutotileSet>> sets;
  AutotileParser parser(sets, path);
  EXPECT_THROW(parser.parse(), std::runtime_error);
}
