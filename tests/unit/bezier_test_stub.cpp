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

// Minimal logging stubs so BezierTest can link math/bezier.cpp without the
// engine console/Squirrel layer. g_log_level is LOG_NONE so log_warning
// short-circuits; the symbol is still required by the linker.

#include <ostream>

#include "util/log.hpp"
#include "video/canvas.hpp"
#include "video/color.hpp"

LogLevel g_log_level = LOG_NONE;
bool g_log_tinygettext = false;

std::ostream& log_warning_f(const char*, int)
{
  static std::ostream s(nullptr);
  return s;
}

// Bezier::draw_curve (never called by the tests) references Canvas::draw_line;
// provide a no-op definition so the translation unit links without dragging in
// the OpenGL/renderer backend.
void Canvas::draw_line(const Vector&, const Vector&, const Color&, int) {}
