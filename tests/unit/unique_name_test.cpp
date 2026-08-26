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

// Coverage for util/unique_name.cpp: make_unique_name() builds
// "gen<unix-time><pointer>" identifiers used for auto-generated objects.

#include <gtest/gtest.h>

#include "util/unique_name.hpp"

#include <set>
#include <string>

TEST(UniqueNameTest, startsWithGenPrefix)
{
  std::string name = make_unique_name("prefix", nullptr);
  // Real semantics: the prefix parameter is ignored; names always start
  // with the literal "gen".
  EXPECT_EQ(name.substr(0, 3), "gen");
}

TEST(UniqueNameTest, distinctPointersProduceDistinctNames)
{
  int a = 0;
  int b = 0;
  std::string na = make_unique_name("", &a);
  std::string nb = make_unique_name("", &b);
  EXPECT_NE(na, nb);
}

TEST(UniqueNameTest, samePointerSameSecondProducesStableName)
{
  int x = 0;
  std::string n1 = make_unique_name("", &x);
  std::string n2 = make_unique_name("", &x);
  EXPECT_EQ(n1, n2);
}

TEST(UniqueNameTest, generatedNamesAreNonEmpty)
{
  EXPECT_FALSE(make_unique_name("", nullptr).empty());
  EXPECT_FALSE(make_unique_name("custom", nullptr).empty());
}
