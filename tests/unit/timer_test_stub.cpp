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

// Minimal stubs so supertux/sequence.cpp links without the engine logging
// layer (util/log.hpp drags in console.hpp -> simplesquirrel VM). Timer itself
// is header-clean; only sequence.cpp's log_warning needs these symbols.

#include <ostream>

// Console symbols referenced by util/log.cpp (never executed at LOG_NONE,
// but the linker needs them). Kept inert on purpose.
#include "supertux/console.hpp"

bool Console::hasFocus() const { return false; }
void Console::open() {}

ConsoleStreamBuffer ConsoleBuffer::s_outputBuffer;
std::ostream ConsoleBuffer::output(&ConsoleBuffer::s_outputBuffer);
void ConsoleBuffer::flush(ConsoleStreamBuffer&) {}

Console::~Console() = default;

// ssq::VM vtable/dtor referenced through Console's member (Currenton<VM>).
#include <simplesquirrel/vm.hpp>
ssq::VM::~VM() = default;
ssq::Object::~Object() = default;
