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

// Coverage for util/reader.cpp: reader_get_layer — the z-pos/layer fallback
// logic used by every game object.
//
// IMPORTANT: ReaderMapping stores REFERENCES to the document and its sexp
// tree. The ReaderDocument MUST outlive the mapping, so each test keeps the
// document alive in a local variable instead of returning a mapping from a
// helper (a returned mapping would dangle — this segfaulted in an earlier
// attempt).

#include <gtest/gtest.h>

#include <sstream>

#include "util/reader.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "video/layer.hpp"

namespace {

int get_layer(const std::string& body, int def)
{
  std::istringstream in("(test-object\n" + body + ")\n");
  auto doc = ReaderDocument::from_stream(in);
  auto mapping = doc.get_root().get_mapping();
  return reader_get_layer(mapping, def);
}

} // namespace

TEST(ReaderGetLayerTest, reads_z_pos)
{
  EXPECT_EQ(42, get_layer("   (z-pos 42)\n", 0));
}

TEST(ReaderGetLayerTest, falls_back_to_legacy_layer_name)
{
  EXPECT_EQ(17, get_layer("   (layer 17)\n", 0));
}

TEST(ReaderGetLayerTest, prefers_z_pos_over_layer)
{
  EXPECT_EQ(5, get_layer("   (z-pos 5)\n   (layer 99)\n", 0));
}

TEST(ReaderGetLayerTest, uses_default_when_neither_present)
{
  EXPECT_EQ(-13, get_layer("   (something-else 1)\n", -13));
}

TEST(ReaderGetLayerTest, clamps_to_gui_layer_limit)
{
  // Values above LAYER_GUI - 100 are clamped (editor inactive in tests).
  EXPECT_EQ(LAYER_GUI - 100, get_layer("   (z-pos 10000)\n", 0));
}

TEST(ReaderGetLayerTest, default_below_limit_is_not_clamped)
{
  EXPECT_EQ(LAYER_GUI - 200, get_layer("   (nothing 0)\n", LAYER_GUI - 200));
}

TEST(ReaderGetLayerTest, negative_z_pos_is_preserved)
{
  // Negative z-pos values (e.g. background layers) are not clamped.
  EXPECT_EQ(LAYER_BACKGROUND0, get_layer("   (z-pos -300)\n", 0));
  EXPECT_EQ(LAYER_BACKGROUND1, get_layer("   (z-pos -200)\n", 0));
}

TEST(ReaderGetLayerTest, z_pos_and_layer_same_value_z_pos_wins)
{
  // When both fields are present with the same value, z-pos still takes
  // precedence (the earlier test already verifies preference, this pins
  // the edge case where the values are identical).
  EXPECT_EQ(10, get_layer("   (z-pos 10)\n   (layer 10)\n", 0));
}

TEST(ReaderGetLayerTest, string_z_pos_falls_back_to_default)
{
  // A non-integer z-pos value causes get<int> to fail, so we fall back to
  // the default.
  EXPECT_EQ(99, get_layer("   (z-pos \"abc\")\n", 99));
}

TEST(ReaderGetLayerTest, string_layer_falls_back_to_default)
{
  // The legacy 'layer' name also throws on a non-integer value; the same
  // try/catch guard must catch it and fall back to the default (regression
  // guard for the reader_get_layer() robustness fix).
  EXPECT_EQ(7, get_layer("   (layer \"xyz\")\n", 7));
}

TEST(ReaderGetLayerTest, negative_layer_is_preserved)
{
  // Legacy 'layer' field can also be negative.
  EXPECT_EQ(-50, get_layer("   (layer -50)\n", 0));
}

/* EOF */
