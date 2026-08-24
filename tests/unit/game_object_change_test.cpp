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

#include "st_assert.hpp"
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

int main(void)
{
  // --- constructor stores all fields -------------------------------------
  {
    UID uid;
    uid = 7u;
    GameObjectChange c("name-x", uid, "old", "new", GameObjectChange::ACTION_MODIFY);
    ST_ASSERT("ctor name", c.name == "name-x");
    ST_ASSERT("ctor uid valid", static_cast<bool>(c.uid));
    ST_ASSERT("ctor data", c.data == "old");
    ST_ASSERT("ctor new_data", c.new_data == "new");
    ST_ASSERT("ctor action", c.action == GameObjectChange::ACTION_MODIFY);
  }

  // --- save -> parse round-trip of all three actions ----------------------
  {
    const std::string text = serialize_set(make_sample_set());
    const std::vector<GameObjectChange> parsed = parse_changes(text);

    ST_ASSERT("three changes parsed", parsed.size() == 3);

    ST_ASSERT("[0] name", parsed[0].name == "tilemap");
    ST_ASSERT("[0] action", parsed[0].action == GameObjectChange::ACTION_MODIFY);
    ST_ASSERT("[0] old data", parsed[0].data == "<old/>");

    ST_ASSERT("[1] name", parsed[1].name == "badguy");
    ST_ASSERT("[1] action create", parsed[1].action == GameObjectChange::ACTION_CREATE);
    ST_ASSERT("[1] empty old data", parsed[1].data == "");

    ST_ASSERT("[2] name", parsed[2].name == "platform");
    ST_ASSERT("[2] action delete", parsed[2].action == GameObjectChange::ACTION_DELETE);
    ST_ASSERT("[2] empty new data", parsed[2].new_data == "");
  }

  // --- REAL SEMANTICS: new_data is NOT serialized --------------------------
  // GameObjectChange::save() writes name/uid/data/action but never
  // new_data, so a parsed change always has an empty new_data regardless
  // of what was stored in memory. Documented behaviour of the undo format;
  // if this ever changes, update this assertion deliberately.
  {
    const std::string text = serialize_set(make_sample_set());
    const std::vector<GameObjectChange> parsed = parse_changes(text);
    for (size_t i = 0; i < parsed.size(); ++i)
      ST_ASSERT("new_data not serialized", parsed[i].new_data.empty());
  }

  // --- UID survives the save/load cycle -----------------------------------
  {
    UID uid;
    uid = 42u;
    std::ostringstream out;
    {
      Writer writer(out);
      writer.start_list("c");
      // Wrap in an "object-change" list, exactly like
      // GameObjectChangeSet::save() does.
      writer.start_list("object-change");
      GameObjectChange("obj", uid, "", "", GameObjectChange::ACTION_MODIFY).save(writer);
      writer.end_list("object-change");
      writer.end_list("c");
    }
    const std::vector<GameObjectChange> parsed = parse_changes(out.str());
    ST_ASSERT("one change parsed", parsed.size() == 1);
    ST_ASSERT("uid valid after roundtrip", static_cast<bool>(parsed[0].uid));
    ST_ASSERT("uid value preserved", parsed[0].uid.get_value() == 42u);
  }

  // --- unknown keys are skipped by the iterator protocol ------------------
  // (mirrors what GameObjectChangeSet's parser filters out)
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
    ST_ASSERT("both keys enumerated", total == 2);
    ST_ASSERT("only object-change matched", matched == 1);
  }

  std::cout << "game_object_change_test: all assertions passed" << std::endl;
  return 0;
}
