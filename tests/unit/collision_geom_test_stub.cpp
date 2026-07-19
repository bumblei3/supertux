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

// Minimal logging stubs so CollisionGeomTest can link collision/collision.cpp
// without the engine console/Squirrel layer. g_log_level is LOG_NONE; log
// symbols are still required by the linker.

#include <ostream>

#include "util/log.hpp"

LogLevel g_log_level = LOG_NONE;
bool g_log_tinygettext = false;

std::ostream& log_debug_f(const char*, int, bool) { static std::ostream s(nullptr); return s; }
std::ostream& log_info_f(const char*, int) { static std::ostream s(nullptr); return s; }
std::ostream& log_warning_f(const char*, int) { static std::ostream s(nullptr); return s; }
std::ostream& log_fatal_f(const char*, int) { static std::ostream s(nullptr); return s; }
