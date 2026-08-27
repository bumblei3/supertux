//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

// Dependency-free coverage for DynamicScopedRef (util/dynamic_scoped_ref.hpp).
// Uses the st_assert harness; the header is self-contained so no extra
// engine linkage is required. DynamicScopedRef only exposes an explicit
// operator bool(), hence the static_cast<bool>(...) around the presence checks.
//
// Converted from ST_ASSERT harness to GoogleTest for better failure diagnostics.

#include <gtest/gtest.h>

#include "util/dynamic_scoped_ref.hpp"

DynamicScopedRef<const int> d_value;

TEST(DynamicScopedRefTest, guard1_binding)
{
  int v1 = 1;
  auto guard1 = d_value.bind(v1);

  EXPECT_TRUE(static_cast<bool>(d_value));
  EXPECT_EQ(*d_value, 1);
  EXPECT_EQ(*d_value.get(), 1);
}

TEST(DynamicScopedRefTest, nested_guard2_binding)
{
  int v1 = 1;
  int v2 = 2;

  auto guard1 = d_value.bind(v1);
  EXPECT_EQ(*d_value, 1);

  {
    auto guard2 = d_value.bind(v2);
    EXPECT_TRUE(static_cast<bool>(d_value));
    EXPECT_EQ(*d_value, 2);
    EXPECT_EQ(*d_value.get(), 2);

    int v3 = 3;
    {
      auto guard3 = d_value.bind(v3);
      EXPECT_EQ(*d_value, 3);
      EXPECT_EQ(*d_value.get(), 3);
    }

    EXPECT_TRUE(static_cast<bool>(d_value));
    EXPECT_EQ(*d_value, 2);
    EXPECT_EQ(*d_value.get(), 2);
  }

  EXPECT_TRUE(static_cast<bool>(d_value));
  EXPECT_EQ(*d_value, 1);
  EXPECT_EQ(*d_value.get(), 1);
}

TEST(DynamicScopedRefTest, unbind_after_all_guards)
{
  int v1 = 1;
  {
    auto guard1 = d_value.bind(v1);
    // empty
  }
  EXPECT_FALSE(d_value);
}