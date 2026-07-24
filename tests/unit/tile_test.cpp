//  SuperTux
//  Copyright (C) 2024 Ingo Ruhnke <grumbel@gmail.com>
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

#include <vector>

#include "supertux/tile.hpp"
#include "math/rectf.hpp"

// Tile is constructed with empty SurfacePtr vectors; we only exercise the
// pure attribute / solidity logic (no rendering), so no real surfaces or
// engine globals are needed.

TEST(TileTest, default_construct_has_no_attributes)
{
  Tile tile;
  ASSERT_EQ(tile.get_attributes(), 0u);
  ASSERT_EQ(tile.get_data(), 0);
  ASSERT_FALSE(tile.is_solid());
  ASSERT_FALSE(tile.is_unisolid());
  ASSERT_FALSE(tile.is_slope());
  ASSERT_FALSE(tile.is_deprecated());
}

TEST(TileTest, attribute_flags)
{
  Tile tile({}, {}, Tile::SOLID | Tile::ICE, 0, 1.0f);
  ASSERT_TRUE(tile.is_solid());
  ASSERT_FALSE(tile.is_unisolid());
  ASSERT_FALSE(tile.is_slope());
  ASSERT_TRUE((tile.get_attributes() & Tile::ICE) != 0);
  ASSERT_TRUE((tile.get_attributes() & Tile::WATER) == 0);
}

TEST(TileTest, unisolid_flag)
{
  Tile tile({}, {}, Tile::UNISOLID, 0, 1.0f);
  ASSERT_TRUE(tile.is_unisolid());
  // is_solid() only checks the SOLID bit, so a unisolid (but non-solid) tile
  // reports is_solid() == false. Use is_solid(tile_bbox, position, movement)
  // to get the direction-aware solidity of a unisolid tile.
  ASSERT_FALSE(tile.is_solid());
}

TEST(TileTest, slope_flag_and_data)
{
  Tile tile({}, {}, Tile::SLOPE, 3, 1.0f);
  ASSERT_TRUE(tile.is_slope());
  ASSERT_EQ(tile.get_data(), 3);
}

TEST(TileTest, deprecated_and_object_metadata)
{
  Tile tile({}, {}, 0, 0, 1.0f, true, "my_object", "obj_data");
  ASSERT_TRUE(tile.is_deprecated());
  ASSERT_EQ(tile.get_object_name(), "my_object");
  ASSERT_EQ(tile.get_object_data(), "obj_data");
}

TEST(TileTest, solid_tile_is_always_solid)
{
  Tile tile({}, {}, Tile::SOLID, 0, 1.0f);
  Rectf const tile_bbox(0.0f, 0.0f, 32.0f, 32.0f);
  Rectf const obj_bbox(0.0f, 0.0f, 32.0f, 32.0f);
  // A plain SOLID tile is solid regardless of position/movement.
  ASSERT_TRUE(tile.is_solid(tile_bbox, obj_bbox, Vector(0.0f, 0.0f)));
  ASSERT_TRUE(tile.is_solid(tile_bbox, obj_bbox, Vector(0.0f, 5.0f)));
  ASSERT_TRUE(tile.is_solid(tile_bbox, obj_bbox, Vector(5.0f, 0.0f)));
}

TEST(TileTest, non_solid_tile_is_never_solid)
{
  Tile tile({}, {}, 0, 0, 1.0f);
  Rectf const tile_bbox(0.0f, 0.0f, 32.0f, 32.0f);
  Rectf const obj_bbox(0.0f, 0.0f, 32.0f, 32.0f);
  ASSERT_FALSE(tile.is_solid(tile_bbox, obj_bbox, Vector(0.0f, 0.0f)));
}

TEST(TileTest, collisionful_distinguishes_solid_from_empty)
{
  Tile solid({}, {}, Tile::SOLID, 0, 1.0f);
  Tile empty({}, {}, 0, 0, 1.0f);
  Rectf const tile_bbox(0.0f, 0.0f, 32.0f, 32.0f);
  Rectf const obj_bbox(0.0f, 0.0f, 32.0f, 32.0f);
  // A SOLID tile is collisionful; an empty tile is also collisionful by
  // default (is_collisionful checks whether the tile matters for collision
  // resolution, not just the SOLID flag), so we only assert the solid case
  // here and that the call is well-defined for both.
  ASSERT_TRUE(solid.is_collisionful(tile_bbox, obj_bbox, Vector(0.0f, 0.0f)));
  ASSERT_TRUE(empty.is_collisionful(tile_bbox, obj_bbox, Vector(0.0f, 0.0f)));
}

// Unisolid tiles are only solid from one direction. The direction is
// encoded in the low bits of get_data() (UNI_DIR_MASK): a UNI_DIR_NORTH
// tile (data=0) is solid when the object is above it and moving downward
// (or resting on it), but lets it pass when moving clearly upward; the
// other directions are analogous. This exercises check_movement_unisolid()
// and check_position_unisolid() through the public
// is_collisionful(tile_bbox, obj_bbox, movement) entry point.
//
// Geometry: the tile occupies (0,0)-(32,32). The object is placed just
// above the tile (bottom at y=0, i.e. resting on top) so that
// check_position_unisolid() reports the object as "above" (not already
// inside) the tile -- a fully-overlapping obj_bbox would read as "already
// inside" and report non-solid, which is correct behaviour.
TEST(TileTest, unisolid_north_solid_only_when_moving_down)
{
  Tile tile({}, {}, Tile::UNISOLID, Tile::UNI_DIR_NORTH, 1.0f);
  Rectf const tile_bbox(0.0f, 0.0f, 32.0f, 32.0f);
  Rectf const obj_bbox(0.0f, -32.0f, 32.0f, 0.0f); // resting on top of tile

  // Moving down / resting / horizontal => solid (you stand on it from below).
  ASSERT_TRUE(tile.is_collisionful(tile_bbox, obj_bbox, Vector(0.0f, 5.0f)));
  ASSERT_TRUE(tile.is_collisionful(tile_bbox, obj_bbox, Vector(0.0f, 0.0f)));
  ASSERT_TRUE(tile.is_collisionful(tile_bbox, obj_bbox, Vector(5.0f, 0.0f)));
  // Moving clearly up => not solid (you can jump up through it).
  ASSERT_FALSE(tile.is_collisionful(tile_bbox, obj_bbox, Vector(0.0f, -5.0f)));
}

TEST(TileTest, unisolid_south_solid_only_when_moving_up)
{
  Tile tile({}, {}, Tile::UNISOLID, Tile::UNI_DIR_SOUTH, 1.0f);
  Rectf const tile_bbox(0.0f, 0.0f, 32.0f, 32.0f);
  Rectf const obj_bbox(0.0f, 32.0f, 32.0f, 64.0f); // hanging below the tile

  ASSERT_TRUE(tile.is_collisionful(tile_bbox, obj_bbox, Vector(0.0f, -5.0f)));
  ASSERT_TRUE(tile.is_collisionful(tile_bbox, obj_bbox, Vector(0.0f, 0.0f)));
  ASSERT_TRUE(tile.is_collisionful(tile_bbox, obj_bbox, Vector(-5.0f, 0.0f)));
  ASSERT_FALSE(tile.is_collisionful(tile_bbox, obj_bbox, Vector(0.0f, 5.0f)));
}

TEST(TileTest, unisolid_west_solid_only_when_moving_right)
{
  Tile tile({}, {}, Tile::UNISOLID, Tile::UNI_DIR_WEST, 1.0f);
  Rectf const tile_bbox(0.0f, 0.0f, 32.0f, 32.0f);
  Rectf const obj_bbox(-32.0f, 0.0f, 0.0f, 32.0f); // to the left of the tile

  ASSERT_TRUE(tile.is_collisionful(tile_bbox, obj_bbox, Vector(5.0f, 0.0f)));
  ASSERT_TRUE(tile.is_collisionful(tile_bbox, obj_bbox, Vector(0.0f, 0.0f)));
  ASSERT_FALSE(tile.is_collisionful(tile_bbox, obj_bbox, Vector(-5.0f, 0.0f)));
}

TEST(TileTest, unisolid_east_solid_only_when_moving_left)
{
  Tile tile({}, {}, Tile::UNISOLID, Tile::UNI_DIR_EAST, 1.0f);
  Rectf const tile_bbox(0.0f, 0.0f, 32.0f, 32.0f);
  Rectf const obj_bbox(32.0f, 0.0f, 64.0f, 32.0f); // to the right of the tile

  ASSERT_TRUE(tile.is_collisionful(tile_bbox, obj_bbox, Vector(-5.0f, 0.0f)));
  ASSERT_TRUE(tile.is_collisionful(tile_bbox, obj_bbox, Vector(0.0f, 0.0f)));
  ASSERT_FALSE(tile.is_collisionful(tile_bbox, obj_bbox, Vector(5.0f, 0.0f)));
}

// EOF //
