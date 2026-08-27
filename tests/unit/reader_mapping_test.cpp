//  SuperTux
//  Copyright (C) 2025 SuperTux contributors
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

// Regression tests for util/reader_mapping.hpp: verify that get() returns
// false (not throw) when a key is missing, and that a type mismatch throws
// a runtime_error carrying file:line + expression context.

#include <gtest/gtest.h>

#include <sstream>

#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

TEST(ReaderMappingTest, get_missing_key_returns_false)
{
  std::istringstream in(
    "(supertux-test\n"
    "   (mybool #t)\n"
    ")\n");

  auto doc = ReaderDocument::from_stream(in);
  auto mapping = doc.get_root().get_mapping();

  bool mybool;
  ASSERT_TRUE(mapping.get("mybool", mybool));
  ASSERT_EQ(true, mybool);

  // A key that was never written must return false (no throw) on plain get().
  int missing = 99;
  ASSERT_FALSE(mapping.get("ghost-key", missing))
      << "get() should return false for a missing key";
  ASSERT_EQ(99, missing) << "get() must not modify the output for a missing key";
}

TEST(ReaderMappingTest, get_existing_key_returns_true)
{
  std::istringstream in(
    "(supertux-test\n"
    "   (present 123)\n"
    ")\n");

  auto doc = ReaderDocument::from_stream(in);
  auto mapping = doc.get_root().get_mapping();

  int present = 0;
  ASSERT_TRUE(mapping.get("present", present));
  ASSERT_EQ(123, present);
}

TEST(ReaderMappingTest, get_type_mismatch_throws_runtime_error)
{
  std::istringstream in(
    "(supertux-test\n"
    "   (kb #t)\n"
    "   (ki 123456789)\n"
    "   (kf 1.125)\n"
    ")\n");

  auto doc = ReaderDocument::from_stream(in);
  auto mapping = doc.get_root().get_mapping();

  bool b;
  int i;
  float f;

  // Reading a key as a different type must throw a runtime_error carrying
  // the expression and file:line context.
  ASSERT_THROW(mapping.get("kb", f), std::runtime_error);
  ASSERT_THROW(mapping.get("ki", b), std::runtime_error);
  ASSERT_THROW(mapping.get("kf", i), std::runtime_error);

  // Reading a scalar key as a mapping must also throw.
  ASSERT_THROW(mapping.get("kf", b), std::runtime_error);
}
