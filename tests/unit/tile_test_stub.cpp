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

// Minimal stubs so supertux/tile.cpp links without pulling in the whole
// engine (DrawingContext/Canvas, Editor, globals). TileTest only exercises
// the attribute / solidity logic; it never calls the draw_* or surface
// methods, but the linker still needs those symbols to exist.

#include "supertux/globals.hpp"   // Config (fwd), g_config, g_game_time
#include "editor/editor.hpp"       // Editor::is_active()
#include "video/canvas.hpp"        // Canvas::draw_*
#include "video/color.hpp"
#include "video/surface_ptr.hpp"
#include "video/paint_style.hpp"
#include "video/blend.hpp"

// --- globals ---
Config* g_config = nullptr;
float g_game_time = 0.0f;

// --- Editor ---
bool Editor::is_active() { return false; }

// --- Canvas draw methods (no-ops) ---
void Canvas::draw_surface(const SurfacePtr&, const Vector&, int) {}
void Canvas::draw_surface(const SurfacePtr&, const Vector&, float, const Color&, const Blend&, int) {}
void Canvas::draw_surface_part(const SurfacePtr&, const Rectf&, const Rectf&, int, const PaintStyle&) {}
void Canvas::draw_surface_scaled(const SurfacePtr&, const Rectf&, int, const PaintStyle&) {}
void Canvas::draw_surface_batch(const SurfacePtr&, std::vector<Rectf>, std::vector<Rectf>, const Color&, int) {}
void Canvas::draw_surface_batch(const SurfacePtr&, std::vector<Rectf>, std::vector<Rectf>, std::vector<float>, const Color&, int) {}
void Canvas::draw_filled_rect(const Rectf&, const Color&, int) {}
void Canvas::draw_filled_rect(const Rectf&, const Color&, float, int) {}
void Canvas::draw_triangle(const Vector&, const Vector&, const Vector&, const Color&, int) {}
void Canvas::draw_line(const Vector&, const Vector&, const Color&, int) {}
void Canvas::draw_inverse_ellipse(const Vector&, const Vector&, const Color&, int) {}
void Canvas::draw_gradient(const Color&, const Color&, int, const GradientDirection&, const Rectf&, const Blend&) {}

// EOF //
