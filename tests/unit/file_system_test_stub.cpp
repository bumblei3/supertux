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
//  along with this program.  if not, see <http://www.gnu.org/licenses/>.

// Link stub so util/file_system.cpp can be compiled into the unit test
// without pulling in the full engine (logging / config / CURL / PhysFS).
// FileSystemTest only exercises the pure path helpers (join/dirname/
// basename/extension/normalize/...); FileSystem::rename() and
// FileSystem::escape_url() reference the engine symbols below, but the test
// never calls them -- the linker just needs the symbols present. These
// stubs are never executed by the test.

#include <ostream>
#include <string>

#include <physfs.h>

#include "util/log.hpp"
#include "supertux/gameconfig.hpp"

// --- logging ---------------------------------------------------------------
LogLevel g_log_level = LOG_NONE;
bool g_log_tinygettext = false;

std::ostream& log_debug_f(const char*, int, bool) { static std::ostream s(nullptr); return s; }
std::ostream& log_info_f(const char*, int) { static std::ostream s(nullptr); return s; }
std::ostream& log_warning_f(const char*, int) { static std::ostream s(nullptr); return s; }
std::ostream& log_fatal_f(const char*, int) { static std::ostream s(nullptr); return s; }

// --- config (only referenced by FileSystem::open_editor, never called) -----
Config* g_config = nullptr;

// --- CURL (only referenced by FileSystem::escape_url, never called) --------
extern "C" {

char* curl_easy_escape(void*, const char*, int)
{
  return nullptr;
}

void curl_free(void*)
{
}

// --- PhysFS (only referenced by FileSystem::rename, never called) ----------

PHYSFS_File* PHYSFS_openRead(const char*)
{
  return nullptr;
}

PHYSFS_File* PHYSFS_openWrite(const char*)
{
  return nullptr;
}

int PHYSFS_close(PHYSFS_File*)
{
  return 0;
}

PHYSFS_sint64 PHYSFS_fileLength(PHYSFS_File*)
{
  return 0;
}

PHYSFS_sint64 PHYSFS_readBytes(PHYSFS_File*, void*, PHYSFS_uint64)
{
  return 0;
}

PHYSFS_sint64 PHYSFS_writeBytes(PHYSFS_File*, const void*, PHYSFS_uint64)
{
  return 0;
}

int PHYSFS_delete(const char*)
{
  return 0;
}

} // extern "C"

/* EOF */
