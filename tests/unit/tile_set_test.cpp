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

// Coverage for supertux/tile_set.cpp: add_tile (resize + duplicate guard),
// get() bounds handling, tilegroup management and deprecated-tile removal.
// Engine-free: TileSet only needs log stubs plus real tile.cpp/color.cpp.

#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "supertux/tile_set.hpp"
#include "supertux/tile.hpp"

TEST(TileSetTest, initial_state_has_empty_tile_zero)
{
  TileSet set("test-tileset");
  // Tile 0 always exists (the "invalid" fallback), everything else not.
  EXPECT_EQ(1u, set.get_max_tileid());
}

TEST(TileSetTest, add_tile_resizes_and_stores)
{
  TileSet set("test-tileset");
  set.add_tile(5, std::make_unique<Tile>());

  EXPECT_EQ(6u, set.get_max_tileid());
  EXPECT_FALSE(set.get(5).is_deprecated());
  // get() on an in-range but unassigned slot returns the fallback tile 0.
  const Tile& fallback = set.get(3);
  EXPECT_EQ(&fallback, &set.get(0));
}

TEST(TileSetTest, get_out_of_range_returns_fallback)
{
  TileSet set("test-tileset");
  const Tile& fallback = set.get(0);
  // Far beyond m_tiles.size(): must not crash, must return fallback.
  const Tile& far = set.get(123456u);
  EXPECT_EQ(&fallback, &far);
}

TEST(TileSetTest, add_tile_rejects_duplicate_id_silently_keeps_first)
{
  TileSet set("test-tileset");
  auto first = std::make_unique<Tile>();
  Tile* first_ptr = first.get();
  

  set.add_tile(2, std::move(first));

  auto second = std::make_unique<Tile>();
  set.add_tile(2, std::move(second)); // must be ignored (with a warning)

  EXPECT_EQ(first_ptr, &set.get(2));
}

TEST(TileSetTest, add_tilegroup_and_remove_deprecated_replaces_with_zero)
{
  TileSet set("test-tileset");
  // Tile 5 will be deprecated (constructor flag); tile 7 stays valid.
  auto dep = std::make_unique<Tile>(std::vector<SurfacePtr>{}, std::vector<SurfacePtr>{},
                                    0, 0, 1.0f, true /* deprecated */);
  set.add_tile(5, std::move(dep));
  set.add_tile(7, std::make_unique<Tile>());

  Tilegroup group;
  group.name = "test-group";
  group.tiles = { 5, 7 };
  set.add_tilegroup(group);

  set.remove_deprecated_tiles();

  // Fetch the stored group back: tile 5 must have been zeroed, 7 kept.
  bool found = false;
  for (const Tilegroup& g : set.get_tilegroups())
  {
    if (g.name == "test-group")
    {
      found = true;
      ASSERT_EQ(2u, g.tiles.size());
      EXPECT_EQ(0, g.tiles[0]); // deprecated -> 0
      EXPECT_EQ(7, g.tiles[1]);
    }
  }
  EXPECT_TRUE(found);
}

/* EOF */
