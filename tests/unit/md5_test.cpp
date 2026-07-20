//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "addon/md5.hpp"

// Known-answer tests (RFC 1321 reference vectors). These pin the digest
// output to canonical MD5 values so any regression in the block transform,
// padding, or finalization shows up as a failed assertion rather than
// silent corruption.

TEST(MD5Test, empty)
{
  std::istringstream empty("");
  EXPECT_EQ(std::string("d41d8cd98f00b204e9800998ecf8427e"),
            std::string(MD5(empty).hex_digest()));
}

TEST(MD5Test, abc)
{
  std::istringstream in("abc");
  EXPECT_EQ(std::string("900150983cd24fb0d6963f7d28e17f72"),
            std::string(MD5(in).hex_digest()));
}

TEST(MD5Test, message_digest)
{
  std::istringstream in("message digest");
  EXPECT_EQ(std::string("f96b697d7cb7938d525a2f31aaf161d0"),
            std::string(MD5(in).hex_digest()));
}

TEST(MD5Test, lowercase_alphabet)
{
  std::istringstream in("abcdefghijklmnopqrstuvwxyz");
  EXPECT_EQ(std::string("c3fcd3d76192e4007dfb496cca67e13b"),
            std::string(MD5(in).hex_digest()));
}

// Multi-block input (> 55 bytes) forces the padding to spill across a
// 64-byte block boundary -- this exercises the block-transform loop and
// the length-encoding in finalize(), which the short vectors above do not.
TEST(MD5Test, multiblock_alphanumeric)
{
  std::istringstream in(
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
  EXPECT_EQ(std::string("d174ab98d277d9f5a5611c2c9f419d9f"),
            std::string(MD5(in).hex_digest()));
}

TEST(MD5Test, helloworld)
{
  std::istringstream helloworld("HelloWorld");
  EXPECT_EQ(std::string("68e109f0f40ca72a15e05cc22786f8e6"),
            std::string(MD5(helloworld).hex_digest()));
}

// Incremental update() must produce the same digest as a single-shot
// construction over the concatenated bytes. This guards the streaming
// path (used by AddonManager to hash files in chunks) against
// off-by-one / buffer-reuse bugs in the block update.
TEST(MD5Test, incremental_equals_single_shot)
{
  const std::string data(
    "The quick brown fox jumps over the lazy dog"); // 43 bytes

  // Single-shot.
  std::istringstream single(data);
  const std::string single_digest = MD5(single).hex_digest();

  // Split into three unequal chunks via update().
  MD5 ctx;
  std::string a = data.substr(0, 10);
  std::string b = data.substr(10, 20);
  std::string c = data.substr(30);
  ctx.update(reinterpret_cast<uint8_t*>(a.data()),
             static_cast<unsigned int>(a.size()));
  ctx.update(reinterpret_cast<uint8_t*>(b.data()),
             static_cast<unsigned int>(b.size()));
  ctx.update(reinterpret_cast<uint8_t*>(c.data()),
             static_cast<unsigned int>(c.size()));
  const std::string incremental_digest = ctx.hex_digest();

  EXPECT_EQ(single_digest, incremental_digest);
  EXPECT_EQ(std::string("9e107d9d372bb6826bd81d3542a419d6"),
            incremental_digest);
}

/* EOF */
