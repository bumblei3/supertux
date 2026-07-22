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
//  along with this program.  If not, see <http://www.gnu/licenses/>.

#include <gtest/gtest.h>

#include "supertux/physic.hpp"

// Time-integration behaviour of Physic::get_movement() over many frames.
//
// get_movement() performs semi-implicit Euler integration that MUTATES the
// velocity in place:
//     v += (a + g) * dt ;  pos += v * dt
// Because the displacement uses the *already updated* velocity, a constant
// acceleration over N steps of dt yields the stepwise sum
//     sum_{k=1..N} (a * k * dt) * dt  =  a * dt^2 * N(N+1)/2
// (not the analytic 0.5*a*t^2, which is the continuous limit). This test
// locks in that exact discrete behaviour -- the real per-frame integration
// the engine uses -- without pulling in the Sector/Level/audio engine stack
// (gravity is disabled and supplied via set_acceleration() instead).
//
// In SuperTux screen coordinates y grows downward, so "gravity" is a positive
// ay and an upward throw is a negative initial vy.
//
// With a = 1000 px/s^2, dt = 0.1 s, N steps:
//   step k displacement = 1000 * 0.1 * k * 0.1 = 10 * k   (px)
//   after N steps total = 10 * N(N+1)/2
//   N=10 (1.0 s): 550 px,  final vy = 1000 px/s
//   N=20 (2.0 s): 2100 px, final vy = 2000 px/s

namespace {

struct IntegrateResult {
  Vector total;
  Vector final_velocity;
};

IntegrateResult integrate(float ax, float ay, float vx0, float vy0,
                          float dt, int frames)
{
  Physic physic;
  physic.enable_gravity(false);
  physic.set_acceleration(ax, ay);
  physic.set_velocity(vx0, vy0);

  IntegrateResult r;
  r.total = Vector(0.0f, 0.0f);
  for (int i = 0; i < frames; ++i)
    r.total += physic.get_movement(dt);
  r.final_velocity = Vector(physic.get_velocity_x(), physic.get_velocity_y());
  return r;
}

} // namespace

TEST(PhysicIntegrationTest, constant_accel_velocity_grows_linearly)
{
  // Final velocity = a * t. ay = 1000, t = 1.0 s -> vy = 1000.
  IntegrateResult const r = integrate(0.0f, 1000.0f, 0.0f, 0.0f, 0.1f, 10);
  EXPECT_NEAR(r.final_velocity.y, 1000.0f, 1e-2f);
  EXPECT_NEAR(r.total.x, 0.0f, 1e-4f);
}

TEST(PhysicIntegrationTest, discrete_position_sum_is_locked_in)
{
  // N=10: total = 10 * 10*11/2 = 550 px (semi-implicit Euler, not 0.5*a*t^2).
  IntegrateResult const r = integrate(0.0f, 1000.0f, 0.0f, 0.0f, 0.1f, 10);
  EXPECT_NEAR(r.total.y, 550.0f, 1e-1f);
}

TEST(PhysicIntegrationTest, position_scales_with_N_Nplus1_over_2)
{
  // N=20: total = 10 * 20*21/2 = 2100 px; final vy = 2000.
  IntegrateResult const r = integrate(0.0f, 1000.0f, 0.0f, 0.0f, 0.1f, 20);
  EXPECT_NEAR(r.total.y, 2100.0f, 1.0f);
  EXPECT_NEAR(r.final_velocity.y, 2000.0f, 1e-2f);
}

TEST(PhysicIntegrationTest, upward_throw_decelerates_then_apex)
{
  // Upward throw vy0 = -500 (screen-up), ay = +1000. Velocity each step:
  //   vy = -500 + 1000*0.1*k ; at k=5 (after 0.5 s) vy = 0 (apex).
  //   displacement step k uses the UPDATED vy, so steps are
  //   -40, -30, -20, -10, 0  -> sum = -100 px (net upward, negative y).
  IntegrateResult const r = integrate(0.0f, 1000.0f, 0.0f, -500.0f, 0.1f, 5);
  EXPECT_NEAR(r.final_velocity.y, 0.0f, 1e-2f);   // apex reached
  EXPECT_NEAR(r.total.y, -100.0f, 1e-1f);          // net upward
}

TEST(PhysicIntegrationTest, horizontal_motion_independent_of_vertical)
{
  // vx constant (ax=0) integrates to vx*t = 100 * 1.0 = 100 px.
  // vy under accel integrates to the same discrete sum as above: 550 px.
  IntegrateResult const r = integrate(0.0f, 1000.0f, 100.0f, 0.0f, 0.1f, 10);
  EXPECT_NEAR(r.total.x, 100.0f, 1e-3f);
  EXPECT_NEAR(r.total.y, 550.0f, 1e-1f);
}

// EOF //
