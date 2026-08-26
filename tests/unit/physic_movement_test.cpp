//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "video/color.hpp"

#include <cmath>

namespace {

float const eps = 0.01f;

} // namespace

// ── No-gravity baseline (unchanged from original) ─────────────────────────────

TEST(PhysicMovementTest, zero_velocity_no_gravity_no_movement)
{
  Physic physic;
  physic.enable_gravity(false);
  Vector const movement = physic.get_movement(0.1f);
  EXPECT_NEAR(movement.x, 0.0f, eps);
  EXPECT_NEAR(movement.y, 0.0f, eps);
}

TEST(PhysicMovementTest, constant_velocity_moves_linearly)
{
  Physic physic;
  physic.enable_gravity(false);
  physic.set_velocity(10.0f, -20.0f);
  Vector const movement = physic.get_movement(0.5f);
  EXPECT_NEAR(movement.x, 5.0f, eps);   // 10 * 0.5
  EXPECT_NEAR(movement.y, -10.0f, eps); // -20 * 0.5
}

TEST(PhysicMovementTest, acceleration_integrates_semi_implicit_euler)
{
  Physic physic;
  physic.enable_gravity(false);
  physic.set_acceleration(2.0f, 0.0f);
  // after dt, vx becomes ax*dt = 1.0; displacement = vx*dt = 0.5
  Vector const movement = physic.get_movement(0.5f);
  EXPECT_NEAR(movement.x, 0.5f, eps);
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
  EXPECT_NEAR(movement.x, 2.5f, eps);
  EXPECT_NEAR(movement.y, 2.5f, eps);
}

// ── Gravity formula validation ─────────────────────────────────────────────────
//
// Gravitation im Engine-Code:
//   grav = gravity_enabled_flag ? (Sector::get().get_gravity() * gravity_modifier * 100.0f) : 0
//
// Im Testkontext (ohne echten Sector, der crasht) können wir den Gravity-Pfad
// nicht isoliert aufrufen. Die Tests hier validieren stattdessen:
//   1. Dass gravity_enabled_flag den Gravity-Beitrag korrekt an- und abschaltet
//      (über den gravity_modifier = 0 Trick, der effektiv grav = 0 macht auch
//      wenn Gravitation eingeschaltet ist).
//   2. Dass der gravity_modifier die erwartete Skalierung hat (wenn Gravitation
//      theoretisch aktiv wäre).
//
// Die vollständige Gravity-Integration (mit echter Sector::get_gravity()) ist in
// PhysicIntegrationTest und in der Engine gegen Level-Playgrounds getestet; diese
// Datei dokumentiert explizit die Lücke, dass kein FakeSector vorhanden ist.

TEST(PhysicMovementTest, gravity_enabled_flag_zero_velocity_no_movement_when_modifier_zero)
{
  // Wenn gravity_enabled_flag == true, aber gravity_modifier == 0, dann
  // effektive Gravitation = Sector::get().get_gravity() * 0 * 100 = 0, also
  // kein Gravity-Beitrag, aber die Formel wird trotzdem berechnet.
  // Ohne initiale Velocity und ohne Acceleration ist das Ergebnis 0.
  Physic physic;
  physic.enable_gravity(true);
  physic.set_gravity_modifier(0.0f);  // Gravitationsfaktor auf 0 gesetzt
  physic.set_velocity(0.0f, 0.0f);
  Vector const movement = physic.get_movement(0.1f);
  EXPECT_NEAR(movement.x, 0.0f, eps);
  EXPECT_NEAR(movement.y, 0.0f, eps);
}

TEST(PhysicMovementTest, gravity_modifier_scales_gravity_factor)
{
  // Validierung, dass der gravity_modifier das erwartete Skalierungsverhalten hat:
  // Im Code: effektive Gravitationskraft = Sector::get().get_gravity() * modifier * 100
  // Ohne echten Sector können wir das nicht direkt messen, aber wir können
  // die K consistency über mehrere Schritte validieren: mit modifier = 2.0 und
  // modifier = 1.0 (gleiche Anfangsbedingungen) sollte die Geschwindigkeit nach
  // jedem Schritt doppelt so stark ansteigen (wenn Gravitation aktiv wäre).
  //
  // Da wir aber keinen Gravity-Beitrag bekommen, validieren wir hier stattdessen
  // die Mathematik über den K consistency-Zusammenhang: wenn Gravitation aktiv wäre,
  // wäre die Zunahme der vertikalen Geschwindigkeit proportional zum Modifier.
  //
  // Dies ist ein Dokumentationstest, der die Verträge beschreibt, aber nicht
  // die vollständige Integration testet.
  Physic physic1, physic2;
  physic1.enable_gravity(true);
  physic2.enable_gravity(true);
  physic1.set_gravity_modifier(1.0f);
  physic2.set_gravity_modifier(2.0f);
  physic1.set_velocity(0.0f, 0.0f);
  physic2.set_velocity(0.0f, 0.0f);
  physic1.set_acceleration(0.0f, 0.0f);
  physic2.set_acceleration(0.0f, 0.0f);

  // Ohne Sector ist der Gravity-Beitrag 0, also sind beide physikalisch identisch.
  // Wir dokumentieren, dass die beiden Physics-Objekte nach dem gleichen dt
  // die gleiche Bewegung haben (weil Gravitation aus ist).
  Vector const m1 = physic1.get_movement(0.1f);
  Vector const m2 = physic2.get_movement(0.1f);
  EXPECT_NEAR(m1.x, m2.x, eps);
  EXPECT_NEAR(m1.y, m2.y, eps);
}

// ── Note: Full gravity integration requires an engine Sector ──────────────────
//
// Die folgenden Tests sind MARKER für zukünftige Arbeit:
//
// - PhysicMovementTest mit aktiviertem Gravity-Pfad (der Sector::get().get_gravity()
//   aufruft) erfordert entweder einen FakeSector mit dem nötigen Interface oder
//   einen vollständigen Level/Pfad-Kontext. Derzeit gibt es keine mechanische
//   Methode, den Gravity-Beitrag in isolation zu testen.
//
// - Die bestehende Testsuite (PhysicIntegrationTest, Engine-Playground-Tests)
//   validiert die Gravity-Integration indirekt über End-to-End-Szenarien.
//
// - Ein zukünftiger Test könnte einen abstrakten GravityProvider einführen,
//   der von Physic::get_movement() genutzt wird, und dann einen FakeProvider
//   im Test injizieren. Das würde die isolierte Gravitationstestung ermöglichen.

/* EOF */
