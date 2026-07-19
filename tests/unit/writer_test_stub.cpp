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

// Minimal logging stubs so WriterTest can link util/writer.cpp without the
// engine console/Squirrel layer. g_log_level is LOG_NONE; log symbols are
// still required by the linker.

#include <ostream>
#include <string>

#include "util/log.hpp"
#include "util/file_system.hpp"

LogLevel g_log_level = LOG_NONE;
bool g_log_tinygettext = false;

std::ostream& log_debug_f(const char*, int, bool) { static std::ostream s(nullptr); return s; }
std::ostream& log_info_f(const char*, int) { static std::ostream s(nullptr); return s; }
std::ostream& log_warning_f(const char*, int) { static std::ostream s(nullptr); return s; }
std::ostream& log_fatal_f(const char*, int) { static std::ostream s(nullptr); return s; }

// ReaderDocument::get_directory() references FileSystem::dirname; provide a
// no-op so the translation unit links without dragging in physfs.
namespace FileSystem {
std::string dirname(const std::string& filename) { return filename; }
} // namespace FileSystem

// The Writer/Reader object-file ctors and UID stream operator are never
// called by this test (we use the ostream ctor and ReaderDocument::from_string),
// but the linker still needs the symbols. Stub them so the translation units
// link without pulling in physfs.
#include "physfs/ofile_stream.hpp"
#include "physfs/ifile_stream.hpp"
#include "util/uid.hpp"

OFileStream::OFileStream(const std::string&) : std::ostream(nullptr) {}
IFileStream::IFileStream(const std::string&) : std::istream(nullptr) {}
std::ostream& operator<<(std::ostream& os, const UID&) { return os; }
