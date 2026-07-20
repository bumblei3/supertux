//  SuperTux
//  Copyright (C) 2024 Ingo Ruhnke <grumbel@gmail.com>
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

#include <gtest/gtest.h>

#include "supertux/physic.hpp"

// NOTE: Physic::get_movement() lives in physic.cpp and calls Sector::get()
// when gravity is enabled, which pulls in the whole engine. The pure state
// logic (velocity / acceleration setters & getters, inversion, reset, gravity
// flag) is header-inline and fully testable in isolation here.

TEST(PhysicTest, default_construct_is_zero)
{
  Physic physic;
  ASSERT_FLOAT_EQ(physic.get_velocity_x(), 0.0f);
  ASSERT_FLOAT_EQ(physic.get_velocity_y(), 0.0f);
  ASSERT_FLOAT_EQ(physic.get_acceleration_x(), 0.0f);
  ASSERT_FLOAT_EQ(physic.get_acceleration_y(), 0.0f);
  ASSERT_TRUE(physic.gravity_enabled());
  ASSERT_FLOAT_EQ(physic.get_gravity_modifier(), 1.0f);
}

TEST(PhysicTest, set_and_get_velocity)
{
  Physic physic;
  physic.set_velocity(3.0f, -4.0f);
  ASSERT_FLOAT_EQ(physic.get_velocity_x(), 3.0f);
  ASSERT_FLOAT_EQ(physic.get_velocity_y(), -4.0f);
  ASSERT_EQ(physic.get_velocity(), Vector(3.0f, -4.0f));
}

TEST(PhysicTest, set_velocity_components)
{
  Physic physic;
  physic.set_velocity_x(7.0f);
  physic.set_velocity_y(2.0f);
  ASSERT_FLOAT_EQ(physic.get_velocity_x(), 7.0f);
  ASSERT_FLOAT_EQ(physic.get_velocity_y(), 2.0f);
}

TEST(PhysicTest, inverse_velocity)
{
  Physic physic;
  physic.set_velocity(5.0f, 6.0f);
  physic.inverse_velocity_x();
  physic.inverse_velocity_y();
  ASSERT_FLOAT_EQ(physic.get_velocity_x(), -5.0f);
  ASSERT_FLOAT_EQ(physic.get_velocity_y(), -6.0f);
}

TEST(PhysicTest, set_and_get_acceleration)
{
  Physic physic;
  physic.set_acceleration(1.0f, -2.0f);
  ASSERT_FLOAT_EQ(physic.get_acceleration_x(), 1.0f);
  ASSERT_FLOAT_EQ(physic.get_acceleration_y(), -2.0f);
  ASSERT_EQ(physic.get_acceleration(), Vector(1.0f, -2.0f));
}

TEST(PhysicTest, set_acceleration_components)
{
  Physic physic;
  physic.set_acceleration_x(9.0f);
  physic.set_acceleration_y(8.0f);
  ASSERT_FLOAT_EQ(physic.get_acceleration_x(), 9.0f);
  ASSERT_FLOAT_EQ(physic.get_acceleration_y(), 8.0f);
}

TEST(PhysicTest, gravity_flag_and_modifier)
{
  Physic physic;
  physic.enable_gravity(false);
  ASSERT_FALSE(physic.gravity_enabled());
  physic.enable_gravity(true);
  ASSERT_TRUE(physic.gravity_enabled());

  physic.set_gravity_modifier(2.5f);
  ASSERT_FLOAT_EQ(physic.get_gravity_modifier(), 2.5f);
}

TEST(PhysicTest, reset_clears_state)
{
  Physic physic;
  physic.set_velocity(5.0f, 6.0f);
  physic.set_acceleration(1.0f, 2.0f);
  physic.enable_gravity(false);
  physic.set_gravity_modifier(3.0f);
  physic.reset();
  ASSERT_FLOAT_EQ(physic.get_velocity_x(), 0.0f);
  ASSERT_FLOAT_EQ(physic.get_velocity_y(), 0.0f);
  ASSERT_FLOAT_EQ(physic.get_acceleration_x(), 0.0f);
  ASSERT_FLOAT_EQ(physic.get_acceleration_y(), 0.0f);
  ASSERT_TRUE(physic.gravity_enabled());   // reset restores gravity flag
  ASSERT_FLOAT_EQ(physic.get_gravity_modifier(), 1.0f);
}

// EOF //
