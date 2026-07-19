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

// Dependency-free coverage for util/utf8_iterator.hpp using the st_assert
// harness (logging stubbed by utf8_iterator_test_stub.cpp). Exercises 1-4 byte
// UTF-8 decoding, iteration/termination, and malformed-sequence handling.
//
// Notably includes a 4-byte sequence (U+1F600) which regression-tests the
// off-by-one in decode_utf8 that read text[p+4] instead of text[p+3].

#include "st_assert.hpp"
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

int main(void)
{
  // Pure ASCII: one code point per byte.
  {
    auto cps = decode("Hi!");
    ST_ASSERT("ascii: count", cps.size() == 3);
    ST_ASSERT("ascii: H", cps.size() == 3 && cps[0] == 0x48);
    ST_ASSERT("ascii: i", cps.size() == 3 && cps[1] == 0x69);
    ST_ASSERT("ascii: !", cps.size() == 3 && cps[2] == 0x21);
  }

  // 2-byte sequence: U+00E9 (e-acute) = 0xC3 0xA9.
  {
    auto cps = decode("\xC3\xA9");
    ST_ASSERT("2-byte: count", cps.size() == 1);
    ST_ASSERT("2-byte: value", cps.size() == 1 && cps[0] == 0x00E9);
  }

  // 3-byte sequence: U+20AC (euro sign) = 0xE2 0x82 0xAC.
  {
    auto cps = decode("\xE2\x82\xAC");
    ST_ASSERT("3-byte: count", cps.size() == 1);
    ST_ASSERT("3-byte: value", cps.size() == 1 && cps[0] == 0x20AC);
  }

  // 4-byte sequence: U+1F600 (grinning face) = 0xF0 0x9F 0x98 0x80.
  // This is the regression test for the text[p+4] off-by-one.
  {
    auto cps = decode("\xF0\x9F\x98\x80");
    ST_ASSERT("4-byte: count", cps.size() == 1);
    ST_ASSERT("4-byte: value (U+1F600)",
              cps.size() == 1 && cps[0] == 0x1F600);
  }

  // Mixed ASCII + multibyte: "A" + euro + "B".
  {
    auto cps = decode("A\xE2\x82\xAC" "B");
    ST_ASSERT("mixed: count", cps.size() == 3);
    ST_ASSERT("mixed: A", cps.size() == 3 && cps[0] == 0x41);
    ST_ASSERT("mixed: euro", cps.size() == 3 && cps[1] == 0x20AC);
    ST_ASSERT("mixed: B", cps.size() == 3 && cps[2] == 0x42);
  }

  // Empty string: the ctor decodes text[0], which for "" is the terminating
  // '\0' (a valid 1-byte NUL). pos advances 0 -> 1, and done() (pos > size(),
  // i.e. 1 > 0) is immediately true, so the loop body never runs. We must NOT
  // call ++it here: operator++ would read past the buffer.
  {
    UTF8Iterator it("");
    ST_ASSERT("empty: first char is NUL", *it == 0);
    ST_ASSERT("empty: done immediately", it.done());
  }

  // Malformed lead byte (a lone continuation byte 0x80) decodes to 0. The ctor
  // leaves pos at 0 (decode threw before advancing). Iterating must terminate
  // rather than loop forever; we advance with a bounded guard and require the
  // iterator to reach done() (it takes two steps: past 0x80, then the implicit
  // string NUL terminator).
  {
    std::string bad = "\x80";
    UTF8Iterator it(bad);
    ST_ASSERT("malformed: chr is 0", *it == 0);
    ST_ASSERT("malformed: not done yet", !it.done());
    int guard = 0;
    while (!it.done() && guard < 8) { ++it; ++guard; }
    ST_ASSERT("malformed: terminates", it.done());
  }
}

/* EOF */
