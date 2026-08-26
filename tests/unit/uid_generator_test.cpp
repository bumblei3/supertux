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

// Coverage for util/uid_generator.cpp: UIDGenerator's magic/id composition.
// Engine-free: only needs the log_warning stub from uid_test_stub.cpp.

#include <gtest/gtest.h>

#include "util/uid_generator.hpp"

namespace {

// The generator packs (magic << 24) | id_counter, so with a single-byte
// magic the id part must stay within 24 bits. We can't read m_id_counter
// directly, but consecutive UIDs must differ in their low bits and share
// the high bits (same generator => same magic).
uint32_t value_of(const UID& uid) { return uid.get_value(); }

} // namespace

TEST(UIDGeneratorTest, firstUidIsValidAndNonZero)
{
  UIDGenerator generator;
  UID uid = generator.next();
  EXPECT_TRUE(static_cast<bool>(uid));
  EXPECT_NE(value_of(uid), 0u);
}

TEST(UIDGeneratorTest, consecutiveUidsShareMagicDifferInId)
{
  UIDGenerator generator;
  uint32_t v1 = value_of(generator.next());
  uint32_t v2 = value_of(generator.next());
  uint32_t v3 = value_of(generator.next());

  // Same generator => same magic in the top 8 bits.
  EXPECT_EQ(v1 >> 24, v2 >> 24);
  EXPECT_EQ(v2 >> 24, v3 >> 24);

  // Ids increment by exactly one.
  EXPECT_EQ(v2 - v1, 1u);
  EXPECT_EQ(v3 - v2, 1u);
}

TEST(UIDGeneratorTest, distinctGeneratorsHaveDistinctMagic)
{
  // s_magic_counter is a global that increments per generator construction,
  // so two generators must never produce colliding uids.
  UIDGenerator g1;
  UIDGenerator g2;
  UIDGenerator g3;

  for (int i = 0; i < 4; ++i)
  {
    uint32_t a = value_of(g1.next());
    uint32_t b = value_of(g2.next());
    uint32_t c = value_of(g3.next());
    EXPECT_NE(a >> 24, b >> 24);
    EXPECT_NE(b >> 24, c >> 24);
    EXPECT_NE(a >> 24, c >> 24);
  }
}

TEST(UIDGeneratorTest, generatedUidsAreValidObjects)
{
  UIDGenerator generator;
  for (int i = 0; i < 100; ++i)
  {
    UID uid = generator.next();
    ASSERT_TRUE(static_cast<bool>(uid));
    ASSERT_NE(value_of(uid), 0u);
  }
}
