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

// Minimal stub so supertux/physic.cpp links without pulling in the whole
// Sector/engine stack. PhysicTest only exercises the header-inline state
// logic plus Physic::Physic()/reset(); it never calls Physic::get_movement(),
// so the only symbol we must satisfy is Sector's static current pointer
// (referenced by the inline Sector::get()/get_gravity() that get_movement()
// uses). Note: under UBSan the vptr check would still pull in `typeinfo for
// Sector`; the PhysicTest target disables -fno-sanitize=vptr on physic.cpp to
// avoid that (see tests/unit/CMakeLists.txt).
#include "supertux/sector.hpp"

Sector* Sector::s_current = nullptr;
