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

// Dependency-free coverage for util/utf8_iterator.hpp using GoogleTest.
// Exercises 1-4 byte UTF-8 decoding, iteration/termination, and
// malformed-sequence handling. Logging stubbed by
// utf8_iterator_test_stub.cpp.
//
// Converted from ST_ASSERT harness to GoogleTest for better failure diagnostics.
//
// Notably includes a 4-byte sequence (U+1F600) which regression-tests the
// off-by-one in decode_utf8 that read text[p+4] instead of text[p+3].

#include <gtest/gtest.h>

#include "util/utf8_iterator.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace {

// Decode a UTF-8 string into a list of code points via UTF8Iterator.
std::vector<uint32_t> decode(const std::string& s)
{
  std::vector<uint32_t> out;
  for (UTF8Iterator it(s); !it.done(); ++it)
    out.push_back(*it);
  return out;
}

} // namespace

// --- Pure ASCII: one code point per byte ----------------------------------

TEST(Utf8IteratorTest, ascii_decode_count_and_values)
{
  std::vector<uint32_t> cps = decode("Hi!");
  ASSERT_EQ(cps.size(), 3u);
  EXPECT_EQ(cps[0], 0x48u);
  EXPECT_EQ(cps[1], 0x69u);
  EXPECT_EQ(cps[2], 0x21u);
}

// --- 2-byte sequence: U+00E9 (e-acute) -----------------------------------

TEST(Utf8IteratorTest, two_byte_sequence_e_acute)
{
  std::vector<uint32_t> cps = decode("\xC3\xA9");
  ASSERT_EQ(cps.size(), 1u);
  EXPECT_EQ(cps[0], 0x00E9u);
}

// --- 3-byte sequence: U+20AC (euro sign) ---------------------------------

TEST(Utf8IteratorTest, three_byte_sequence_euro)
{
  std::vector<uint32_t> cps = decode("\xE2\x82\xAC");
  ASSERT_EQ(cps.size(), 1u);
  EXPECT_EQ(cps[0], 0x20ACu);
}

// --- 4-byte sequence: U+1F600 (grinning face) ---------------------------

// Regression test for the text[p+4] off-by-one (should read p+3).
TEST(Utf8IteratorTest, four_byte_sequence_grinning_face)
{
  std::vector<uint32_t> cps = decode("\xF0\x9F\x98\x80");
  ASSERT_EQ(cps.size(), 1u);
  EXPECT_EQ(cps[0], 0x1F600u);
}

// --- Mixed ASCII + multibyte ---------------------------------------------

TEST(Utf8IteratorTest, mixed_ascii_and_multibyte)
{
  std::vector<uint32_t> cps = decode("A" "\xE2\x82\xAC" "B");
  ASSERT_EQ(cps.size(), 3u);
  EXPECT_EQ(cps[0], 0x41u);
  EXPECT_EQ(cps[1], 0x20ACu);
  EXPECT_EQ(cps[2], 0x42u);
}

// --- Empty string --------------------------------------------------------

// The ctor decodes text[0], which for "" is the terminating '\0' (a valid
// 1-byte NUL). pos advances 0 -> 1, and done() (pos > size(), i.e. 1 > 0)
// is immediately true, so the loop body never runs. We must NOT call ++it
// here: operator++ would read past the buffer.
TEST(Utf8IteratorTest, empty_string_first_char_is_nul_and_done)
{
  std::string empty = "";
  UTF8Iterator it(empty);
  EXPECT_EQ(*it, 0u);
  EXPECT_TRUE(it.done());
}

// --- Malformed lead byte (lone continuation byte) -----------------------

// A lone continuation byte 0x80 decodes to 0. The ctor leaves pos at 0
// (decode threw before advancing). Iterating must terminate rather than loop
// forever; it takes two steps: past 0x80, then the implicit string NUL
// terminator.
TEST(Utf8IteratorTest, malformed_lone_continuation_byte_terminates)
{
  std::string bad = "\x80";
  UTF8Iterator it(bad);
  EXPECT_EQ(*it, 0u);
  EXPECT_FALSE(it.done());

  int guard = 0;
  while (!it.done() && guard < 8) {
    ++it;
    ++guard;
  }
  EXPECT_TRUE(it.done());
  EXPECT_LE(guard, 8);
}