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

// Coverage for util/currenton.hpp: the "current instance" registry.
// Header-only template, engine-free.

#include <gtest/gtest.h>

#include "util/currenton.hpp"

namespace {

class Widget : public Currenton<Widget>
{
public:
  int id = 0;
};

} // namespace

TEST(CurrentonTest, no_instance_current_returns_null)
{
  // Fresh translation unit state per binary; ensure clean slate.
  ASSERT_EQ(nullptr, Widget::current());
}

TEST(CurrentonTest, construction_registers_as_current)
{
  {
    Widget w;
    w.id = 1;
    ASSERT_EQ(&w, Widget::current());
    EXPECT_EQ(1, Widget::current()->id);
  }
}

TEST(CurrentonTest, destruction_clears_current)
{
  {
    Widget w;
    ASSERT_NE(nullptr, Widget::current());
  }
  EXPECT_EQ(nullptr, Widget::current());
}

TEST(CurrentonTest, newer_instance_replaces_older)
{
  Widget first;
  first.id = 1;

  {
    Widget second;
    second.id = 2;
    EXPECT_EQ(&second, Widget::current());
    EXPECT_EQ(2, Widget::current()->id);
  }

  // After the newer one dies, current() points back at... nothing (it was
  // overwritten). Documented behaviour: last-constructed wins permanently
  // until IT is destroyed, then null (not restored to older instance).
  EXPECT_EQ(nullptr, Widget::current());

  // The first instance is still alive but no longer reachable via current().
  (void)first.id; // silence unused warning while keeping lifetime
}

TEST(CurrentonTest, derived_class_shares_current_slot)
{
  class Derived : public Widget {};

  Derived d;
  // CRTP static is per-instantiation: Derived has its own slot.
  EXPECT_EQ(&d, Derived::current());
}

/* EOF */
