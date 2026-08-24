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

// Coverage for util/uid.cpp: stream output operator and std::hash<UID>.
// These are defined in uid.cpp which UIDTest does NOT link (it only links
// uid_generator.cpp), so the operators were untested until now.

#include <gtest/gtest.h>

#include <sstream>
#include <unordered_set>

#include "util/uid.hpp"

TEST(UIDStreamHashTest, stream_operator_prints_numeric_value)
{
  UID uid;
  uid = 0x01000005u;
  std::ostringstream out;
  out << uid;
  EXPECT_EQ("16777221", out.str()); // decimal representation
}

TEST(UIDStreamHashTest, hash_matches_value)
{
  UID uid;
  uid = 1234567u;
  std::hash<UID> hasher;
  EXPECT_EQ(1234567u, hasher(uid));
}

TEST(UIDStreamHashTest, uids_work_as_unordered_set_keys)
{
  UID a; a = 1u;
  UID b; b = 2u;

  std::unordered_set<UID> set;
  set.insert(a);
  set.insert(b);

  EXPECT_EQ(2u, set.size());
  EXPECT_NE(set.find(a), set.end());
  EXPECT_NE(set.find(b), set.end());

  UID dup; dup = 1u;
  set.insert(dup);
  EXPECT_EQ(2u, set.size());
}

/* EOF */
