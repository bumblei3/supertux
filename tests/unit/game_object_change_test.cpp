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

// Round-trip coverage for supertux/game_object_change.cpp: the undo/redo
// data model written via Writer and parsed back through ReaderIterator,
// exactly the way GameObjectChangeSet's own parser reads it. Uses the same
// ostream-Writer / ReaderDocument::from_string setup as WriterTest; logging
// is stubbed (writer_test_stub.cpp).

#include <gtest/gtest.h>

#include "supertux/game_object_change.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

#include <sstream>

namespace {

std::string serialize_set(const GameObjectChangeSet& set)
{
  std::ostringstream out;
  {
    Writer writer(out);
    writer.start_list("supertux-changes");
    set.save(writer);
    writer.end_list("supertux-changes");
  }
  return out.str();
}

std::vector<GameObjectChange> parse_changes(const std::string& text)
{
  auto doc = ReaderDocument::from_string(text);
  ReaderMapping root = doc.get_root().get_mapping();

  std::vector<GameObjectChange> changes;
  auto iter = root.get_iter();
  while (iter.next())
  {
    if (iter.get_key() != "object-change")
      continue;
    changes.emplace_back(iter.as_mapping());
  }
  return changes;
}

GameObjectChangeSet make_sample_set()
{
  UID uid1; uid1 = 7u;
  UID uid2; uid2 = 9u;
  UID uid3; uid3 = 11u;
  std::vector<GameObjectChange> changes;
  changes.emplace_back("tilemap", uid1, "<old/>", "<new/>", GameObjectChange::ACTION_MODIFY);
  changes.emplace_back("badguy", uid2, "", "<pos x=\"1\" y=\"2\"/>", GameObjectChange::ACTION_CREATE);
  changes.emplace_back("platform", uid3, "<path/>", "", GameObjectChange::ACTION_DELETE);
  return GameObjectChangeSet(UID(), std::move(changes));
}

} // namespace

// ── Constructor field storage ──────────────────────────────────────────────────

TEST(GameObjectChangeTest, ctor_stores_all_fields)
{
  UID uid; uid = 7u;
  GameObjectChange c("name-x", uid, "old", "new", GameObjectChange::ACTION_MODIFY);
  EXPECT_EQ(c.name, "name-x");
  EXPECT_TRUE(static_cast<bool>(c.uid));
  EXPECT_EQ(c.data, "old");
  EXPECT_EQ(c.new_data, "new");
  EXPECT_EQ(c.action, GameObjectChange::ACTION_MODIFY);
}

TEST(GameObjectChangeTest, create_action_stores_fields)
{
  UID uid; uid = 1u;
  GameObjectChange c("badguy", uid, "", "<pos x=\"1\" y=\"2\"/>", GameObjectChange::ACTION_CREATE);
  EXPECT_EQ(c.name, "badguy");
  EXPECT_TRUE(static_cast<bool>(c.uid));
  EXPECT_EQ(c.data, "");
  EXPECT_EQ(c.new_data, "<pos x=\"1\" y=\"2\"/>");
  EXPECT_EQ(c.action, GameObjectChange::ACTION_CREATE);
}

TEST(GameObjectChangeTest, delete_action_stores_fields)
{
  UID uid; uid = 2u;
  GameObjectChange c("platform", uid, "<path/>", "", GameObjectChange::ACTION_DELETE);
  EXPECT_EQ(c.name, "platform");
  EXPECT_EQ(c.data, "<path/>");
  EXPECT_EQ(c.new_data, "");
  EXPECT_EQ(c.action, GameObjectChange::ACTION_DELETE);
}

// ── Round-trip through serialize / parse ───────────────────────────────────────

TEST(GameObjectChangeTest, three_changes_roundtrip)
{
  const std::string text = serialize_set(make_sample_set());
  const std::vector<GameObjectChange> parsed = parse_changes(text);
  ASSERT_EQ(parsed.size(), 3);

  EXPECT_EQ(parsed[0].name, "tilemap");
  EXPECT_EQ(parsed[0].action, GameObjectChange::ACTION_MODIFY);
  EXPECT_EQ(parsed[0].data, "<old/>");

  EXPECT_EQ(parsed[1].name, "badguy");
  EXPECT_EQ(parsed[1].action, GameObjectChange::ACTION_CREATE);
  EXPECT_EQ(parsed[1].data, "");

  EXPECT_EQ(parsed[2].name, "platform");
  EXPECT_EQ(parsed[2].action, GameObjectChange::ACTION_DELETE);
  EXPECT_EQ(parsed[2].new_data, "");
}

TEST(GameObjectChangeTest, new_data_not_serialized)
{
  // GameObjectChange::save() writes name/uid/data/action but never new_data,
  // so a parsed change always has empty new_data. Documented behaviour of the
  // undo format; if this ever changes, update this assertion deliberately.
  const std::string text = serialize_set(make_sample_set());
  const std::vector<GameObjectChange> parsed = parse_changes(text);
  for (size_t i = 0; i < parsed.size(); ++i)
    EXPECT_TRUE(parsed[i].new_data.empty());
}

TEST(GameObjectChangeTest, uid_survives_roundtrip)
{
  UID uid; uid = 42u;
  std::ostringstream out;
  {
    Writer writer(out);
    writer.start_list("c");
    writer.start_list("object-change");
    GameObjectChange("obj", uid, "", "", GameObjectChange::ACTION_MODIFY).save(writer);
    writer.end_list("object-change");
    writer.end_list("c");
  }
  const std::vector<GameObjectChange> parsed = parse_changes(out.str());
  ASSERT_EQ(parsed.size(), 1);
  EXPECT_TRUE(static_cast<bool>(parsed[0].uid));
  EXPECT_EQ(parsed[0].uid.get_value(), 42u);
}

TEST(GameObjectChangeTest, uid_zero_survives_roundtrip)
{
  UID uid; uid = 0u;
  std::ostringstream out;
  {
    Writer writer(out);
    writer.start_list("c");
    writer.start_list("object-change");
    GameObjectChange("zero", uid, "old", "new", GameObjectChange::ACTION_MODIFY).save(writer);
    writer.end_list("object-change");
    writer.end_list("c");
  }
  const std::vector<GameObjectChange> parsed = parse_changes(out.str());
  ASSERT_EQ(parsed.size(), 1);
  EXPECT_EQ(parsed[0].name, "zero");
  EXPECT_EQ(parsed[0].uid.get_value(), 0u);
  EXPECT_EQ(parsed[0].action, GameObjectChange::ACTION_MODIFY);
}

TEST(GameObjectChangeTest, uid_value_roundtrip)
{
  // UID serializes as its integer value; we exercise a few distinct non-zero
  // values. (UID=0 serializes as empty string, which Reader cannot parse back —
  // that is a known format limitation, not a test target.)
  for (uint32_t v : {1u, 100u, 12345u}) {
    UID uid; uid = v;
    std::ostringstream out;
    {
      Writer writer(out);
      writer.start_list("c");
      writer.start_list("object-change");
      GameObjectChange("x", uid, "d", "nd",
                        GameObjectChange::ACTION_MODIFY).save(writer);
      writer.end_list("object-change");
      writer.end_list("c");
    }
    const std::vector<GameObjectChange> parsed = parse_changes(out.str());
    ASSERT_EQ(parsed.size(), 1);
    EXPECT_EQ(parsed[0].uid.get_value(), v) << "failed for v=" << v;
  }
}

// ── Unknown-key filtering ───────────────────────────────────────────────────────

TEST(GameObjectChangeTest, unknown_keys_skipped_by_iterator)
{
  std::string text =
    "(changes\n"
    "  (junk-key (whatever 1))\n"
    "  (object-change (name \"a\") (action 0))\n"
    ")\n";
  auto doc = ReaderDocument::from_string(text);
  ReaderMapping root = doc.get_root().get_mapping();
  size_t total = 0, matched = 0;
  auto iter = root.get_iter();
  while (iter.next())
  {
    ++total;
    if (iter.get_key() == "object-change")
      ++matched;
  }
  EXPECT_EQ(total, 2);
  EXPECT_EQ(matched, 1);
}

// ── Empty-set round-trip ────────────────────────────────────────────────────────

TEST(GameObjectChangeTest, empty_set_roundtrip)
{
  UID uid;
  GameObjectChangeSet empty(uid, std::vector<GameObjectChange>{});
  const std::string text = serialize_set(empty);
  const std::vector<GameObjectChange> parsed = parse_changes(text);
  EXPECT_TRUE(parsed.empty());
}

// ── Single change ───────────────────────────────────────────────────────────────

TEST(GameObjectChangeTest, single_change_roundtrip)
{
  UID uid; uid = 5u;
  GameObjectChangeSet set(UID(),
    {GameObjectChange("single", uid, "data", "new_data",
                      GameObjectChange::ACTION_MODIFY)});
  const std::string text = serialize_set(set);
  const std::vector<GameObjectChange> parsed = parse_changes(text);
  ASSERT_EQ(parsed.size(), 1);
  EXPECT_EQ(parsed[0].name, "single");
  EXPECT_EQ(parsed[0].action, GameObjectChange::ACTION_MODIFY);
  EXPECT_EQ(parsed[0].data, "data");
  EXPECT_TRUE(parsed[0].new_data.empty());
}

// ── UID get_value edge cases ────────────────────────────────────────────────────

TEST(GameObjectChangeTest, uid_one_roundtrip)
{
  UID uid; uid = 1u;
  std::ostringstream out;
  {
    Writer writer(out);
    writer.start_list("c");
    writer.start_list("object-change");
    GameObjectChange("one", uid, "d", "nd",
                      GameObjectChange::ACTION_MODIFY).save(writer);
    writer.end_list("object-change");
    writer.end_list("c");
  }
  const std::vector<GameObjectChange> parsed = parse_changes(out.str());
  ASSERT_EQ(parsed.size(), 1);
  EXPECT_EQ(parsed[0].uid.get_value(), 1u);
}

/* EOF */
