//  SuperTux
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

#include "physfs/physfs_sdl.hpp"

#include <physfs.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_iostream.h>

#include "physfs/util.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <assert.h>
#include <stdio.h>

namespace {

/** Resolve a PHYSFS path that already exists to a real filesystem path. */
std::string real_path_for_existing(const std::string& filename)
{
  const char* path = PHYSFS_getRealDir(filename.c_str());
  if (!path) {
    std::stringstream msg;
    msg << "File '" << filename << "' doesn't exist in any search path";
    throw std::runtime_error(msg.str());
  }
  return FileSystem::join(path, filename);
}

/** Resolve a PHYSFS write path (may not exist yet) under the write dir. */
std::string real_path_for_write(const std::string& filename)
{
  const char* write_dir = PHYSFS_getWriteDir();
  if (!write_dir) {
    throw std::runtime_error("PHYSFS write directory is not set");
  }
  return FileSystem::join(write_dir, filename);
}

} // namespace

SDL_IOStream* get_physfs_SDLRWops(const std::string& filename)
{
  // check this as PHYSFS seems to be buggy and still returns a
  // valid pointer in this case
  if (filename.empty()) {
    throw std::runtime_error("Couldn't open file: empty filename");
  }

  auto full_path = real_path_for_existing(filename);
  SDL_IOStream* ops = SDL_IOFromFile(full_path.c_str(), "rb");
  if (!ops) {
    std::stringstream msg;
    msg << "Couldn't open '" << filename << "' (" << full_path << "): "
        << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
  return ops;
}

SDL_IOStream* get_writable_physfs_SDLRWops(const std::string& filename)
{
  // check this as PHYSFS seems to be buggy and still returns a
  // valid pointer in this case
  if (filename.empty()) {
    throw std::runtime_error("Couldn't open file: empty filename");
  }

  // PHYSFS_getRealDir only works for *existing* files. Screenshots and
  // other writers create new files, so resolve against the write dir.
  auto full_path = real_path_for_write(filename);
  SDL_IOStream* ops = SDL_IOFromFile(full_path.c_str(), "wb");
  if (!ops) {
    std::stringstream msg;
    msg << "Couldn't open '" << filename << "' (" << full_path
        << ") for writing: " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
  return ops;
}
