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

// Covers the out-of-line Physic::get_movement() integration math
// (semi-implicit Euler). With gravity disabled, get_movement() never calls
// Sector::get(), so no engine/Sector linkage is required -- this isolates the
// velocity/acceleration integration, which is the bulk of the movement logic.
// (The gravity-modifier path that touches Sector::get_gravity() is left to a
// future Sector-aware harness.)

TEST(PhysicMovementTest, zero_velocity_no_gravity_no_movement)
{
  Physic physic;
  physic.enable_gravity(false);
  Vector const movement = physic.get_movement(0.1f);
  ASSERT_FLOAT_EQ(movement.x, 0.0f);
  ASSERT_FLOAT_EQ(movement.y, 0.0f);
}

TEST(PhysicMovementTest, constant_velocity_moves_linearly)
{
  Physic physic;
  physic.enable_gravity(false);
  physic.set_velocity(10.0f, -20.0f);
  Vector const movement = physic.get_movement(0.5f);
  ASSERT_FLOAT_EQ(movement.x, 5.0f);   // 10 * 0.5
  ASSERT_FLOAT_EQ(movement.y, -10.0f); // -20 * 0.5
}

TEST(PhysicMovementTest, acceleration_integrates_semi_implicit_euler)
{
  Physic physic;
  physic.enable_gravity(false);
  physic.set_acceleration(2.0f, 0.0f);
  // after dt, vx becomes ax*dt = 1.0; displacement = vx*dt = 0.5
  Vector const movement = physic.get_movement(0.5f);
  ASSERT_FLOAT_EQ(movement.x, 0.5f);
}

TEST(PhysicMovementTest, velocity_and_acceleration_combine)
{
  Physic physic;
  physic.enable_gravity(false);
  physic.set_velocity(4.0f, 0.0f);
  physic.set_acceleration(2.0f, 10.0f);
  // vx = 4 + 2*0.5 = 5 -> dx = 5*0.5 = 2.5
  // vy = 0 + 10*0.5 = 5 -> dy = 5*0.5 = 2.5
  Vector const movement = physic.get_movement(0.5f);
  ASSERT_FLOAT_EQ(movement.x, 2.5f);
  ASSERT_FLOAT_EQ(movement.y, 2.5f);
}

// EOF //
