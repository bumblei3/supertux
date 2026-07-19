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

// Minimal stubs for the logging symbols util/utf8_iterator.cpp references
// (via util/log.hpp) so UTF8IteratorTest stays a pure-logic test without
// linking the engine logging layer (which drags in the console/Squirrel).
//
// g_log_level defaults to LOG_NONE, so the log_debug macro short-circuits and
// log_debug_f is never actually invoked -- but the linker still needs a symbol.

#include <ostream>

#include "util/log.hpp"

LogLevel g_log_level = LOG_NONE;
bool g_log_tinygettext = false;

std::ostream& log_debug_f(const char*, int, bool)
{
  static std::ostream null_stream(nullptr);
  return null_stream;
}
