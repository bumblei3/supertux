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

// Coverage for supertux/autotile.cpp: AutotileMask, Autotile (mask matching,
// deterministic pick_tile, is_amongst) and AutotileSet (neighbour bitmask
// assembly for corner- and edge-based sets). Engine-free; only log.hpp is
// stubbed.

#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "supertux/autotile.hpp"

namespace {

AutotileMask make_mask(uint8_t mask, bool center = true)
{
  return AutotileMask(mask, center);
}

std::vector<AutotileMask> single_mask(uint8_t mask, bool center = true)
{
  return { make_mask(mask, center) };
}

} // namespace

TEST(AutotileMaskTest, matches_requires_exact_mask_and_center)
{
  AutotileMask m(0x0F, true);
  EXPECT_TRUE(m.matches(0x0F, true));
  EXPECT_FALSE(m.matches(0x0F, false)); // center differs
  EXPECT_FALSE(m.matches(0x07, true));  // mask differs
}

TEST(AutotileTest, matches_any_of_multiple_masks)
{
  Autotile tile(1u,
                {},
                single_mask(0x01) /* then */ , true);
  // Rebuild with two masks via the public constructor:
  std::vector<AutotileMask> masks = { make_mask(0x01), make_mask(0x02) };
  Autotile tile2(1u, {}, masks, true);

  EXPECT_TRUE(tile2.matches(0x01, true));
  EXPECT_TRUE(tile2.matches(0x02, true));
  EXPECT_FALSE(tile2.matches(0x04, true));
}

TEST(AutotileTest, pick_tile_returns_base_without_alternatives)
{
  Autotile tile(42u, {}, single_mask(0x01), true);
  EXPECT_EQ(42u, tile.pick_tile(7, 3));
  EXPECT_EQ(42u, tile.pick_tile(100, 200));
}

TEST(AutotileTest, pick_tile_is_deterministic_per_position)
{
  // Same position must always pick the same tile (the hash depends on x/y).
  // Weight 0.25 each: the pseudo-random value (v/256, in [0,1)) is decremented
  // by the first alt's weight; only positions with v >= 64 fall through to
  // the second alternative or the base tile.
  Autotile::AltConditions cond{};
  cond.weight = 0.25f;
  std::vector<std::pair<uint32_t, Autotile::AltConditions>> alts = {
    {100u, cond}, {200u, cond}
  };
  Autotile tile(1u, alts, single_mask(0x01), true);

  EXPECT_EQ(tile.pick_tile(5, 5), tile.pick_tile(5, 5));
  EXPECT_EQ(tile.pick_tile(-3, 12), tile.pick_tile(-3, 12));

  // Many positions should not all resolve to the same alternative.
  bool saw_different = false;
  uint32_t first = tile.pick_tile(0, 0);
  for (int x = 0; x < 16 && !saw_different; ++x)
    for (int y = 0; y < 16 && !saw_different; ++y)
      if (tile.pick_tile(x, y) != first)
        saw_different = true;
  EXPECT_TRUE(saw_different);
}

TEST(AutotileTest, pick_tile_respects_period_conditions)
{
  Autotile::AltConditions cond{};
  cond.period_x = {3, 0}; // only when x % 3 == 0
  std::vector<std::pair<uint32_t, Autotile::AltConditions>> alts = {{99u, cond}};
  Autotile tile(1u, alts, single_mask(0x01), true);

  EXPECT_EQ(99u, tile.pick_tile(6, 0));   // 6 % 3 == 0 -> alternative
  EXPECT_EQ(1u,  tile.pick_tile(7, 0));   // 7 % 3 != 0 -> base tile
}

TEST(AutotileTest, is_amongst_checks_base_and_alternatives)
{
  Autotile::AltConditions cond{};
  std::vector<std::pair<uint32_t, Autotile::AltConditions>> alts = {{77u, cond}};
  Autotile tile(1u, alts, single_mask(0x01), true);

  EXPECT_TRUE(tile.is_amongst(1u));
  EXPECT_TRUE(tile.is_amongst(77u));
  EXPECT_FALSE(tile.is_amongst(2u));
}

TEST(AutotileTest, get_first_mask_returns_first_or_zero)
{
  Autotile no_masks(1u, {}, {}, true);
  EXPECT_EQ(0u, no_masks.get_first_mask());

  Autotile with_masks(1u, {}, single_mask(0xAB), true);
  EXPECT_EQ(0xAB, with_masks.get_first_mask());
}

TEST(AutotileSetTest, edge_based_bitmask_encoding)
{
  // Non-corner set: neighbours map to bits bottom_right=0x01, bottom=0x02,
  // bottom_left=0x04, right=0x08, left=0x10, top_right=0x20, top=0x40,
  // top_left=0x80.
  std::vector<Autotile*> tiles = {
    new Autotile(50u, {}, single_mask(0x10 | 0x40, false), true) // left+top edge
  };
  AutotileSet set(tiles, 9u, "edge-set", false);

  // center=false here: the mask matches with center=false (the AutotileMask's
  // center flag must be false too, matching an edge tile).
  uint32_t result = set.get_autotile(
    0u,
    false, true, false,    // top_left, top, top_right
    true,  false, false,   // left, center, right
    false, false, false,   // bottom_left, bottom, bottom_right
    0, 0);
  EXPECT_EQ(50u, result);
}

TEST(AutotileSetTest, corner_based_ignores_edge_neighbours_and_center_flag)
{
  // Corner set: only corners matter (bottom_right=0x01, bottom_left=0x02,
  // top_right=0x04, top_left=0x08) and center is forced to true.
  std::vector<Autotile*> tiles = {
    new Autotile(60u, {}, single_mask(0x08), true) // top_left corner
  };
  AutotileSet set(tiles, 9u, "corner-set", true);

  uint32_t result = set.get_autotile(
    0u,
    true, false, false,    // top_left set
    false, false, false,   // edges ignored in corner mode
    false, false, false,
    0, 0);
  EXPECT_EQ(60u, result);
}

TEST(AutotileSetTest, unmatched_center_falls_back_to_default_tile)
{
  std::vector<Autotile*> tiles = {
    new Autotile(50u, {}, single_mask(0xFF), true)
  };
  AutotileSet set(tiles, 9u, "fallback", false);

  // Isolated tile: no neighbours, center=true -> no mask matches -> default.
  uint32_t result = set.get_autotile(
    0u,
    false, false, false,
    false, true, false,
    false, false, false,
    0, 0);
  EXPECT_EQ(9u, result);
}

TEST(AutotileSetTest, unmatched_non_center_returns_zero)
{
  std::vector<Autotile*> tiles = {
    new Autotile(50u, {}, single_mask(0xFF), true)
  };
  AutotileSet set(tiles, 9u, "zero-fallback", false);

  uint32_t result = set.get_autotile(
    0u,
    false, false, false,
    false, false, false,   // center=false -> 0 on miss
    false, false, false,
    0, 0);
  EXPECT_EQ(0u, result);
}

TEST(AutotileSetTest, is_member_checks_base_alts_and_default)
{
  Autotile::AltConditions cond{};
  std::vector<Autotile*> tiles = {
    new Autotile(1u, {{77u, cond}}, single_mask(0x01), true)
  };
  AutotileSet set(tiles, 9u, "membership", false);

  EXPECT_TRUE(set.is_member(1u));   // base id
  EXPECT_TRUE(set.is_member(77u));  // alternative id
  EXPECT_TRUE(set.is_member(9u));   // default counts as member
  EXPECT_FALSE(set.is_member(2u));
  EXPECT_FALSE(set.is_member(0u));  // default==9, so 0 is not a member
}

TEST(AutotileSetTest, is_solid_tracks_tile_solidity)
{
  // Real semantics: is_solid(id) checks the matching Autotile's solidity;
  // the default tile id reports solid only if m_default != 0.
  Autotile::AltConditions cond{};
  std::vector<Autotile*> tiles = {
    new Autotile(1u, {{77u, cond}}, single_mask(0x01), true),   // solid
    new Autotile(2u, {}, single_mask(0x02, false), false)       // non-solid
  };
  AutotileSet set(tiles, 9u, "solids", false);

  EXPECT_TRUE(set.is_solid(1u));
  EXPECT_TRUE(set.is_solid(77u));   // alt id inherits the autotile's solidity
  EXPECT_FALSE(set.is_solid(2u));
  EXPECT_TRUE(set.is_solid(9u));    // default id counts as solid
  EXPECT_FALSE(set.is_solid(0u));   // default==9 -> 0 not solid
}

TEST(AutotileSetTest, get_mask_from_tile_returns_first_mask_or_zero)
{
  std::vector<Autotile*> tiles = {
    new Autotile(1u, {}, single_mask(0xAB), true),
    new Autotile(2u, {}, single_mask(0x0C, false), true)
  };
  AutotileSet set(tiles, 9u, "masks", false);

  EXPECT_EQ(0xAB, set.get_mask_from_tile(1u));
  EXPECT_EQ(0x0C, set.get_mask_from_tile(2u));
  EXPECT_EQ(0x00, set.get_mask_from_tile(99u)); // unknown -> 0
}

/* EOF */
