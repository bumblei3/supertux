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

// Minimal stubs so AnchorPointTest can link math/anchor_point.cpp without the
// engine's logging or tinygettext runtime. g_dictionary_manager stays null, so
// the inline _() in gettext.hpp returns messages verbatim; g_log_level is
// LOG_NONE so the log_warning macro short-circuits (symbol still needed).

#include <memory>
#include <ostream>

#include "util/log.hpp"
#include "util/gettext.hpp"

LogLevel g_log_level = LOG_NONE;
bool g_log_tinygettext = false;

std::unique_ptr<tinygettext::DictionaryManager> g_dictionary_manager;

std::ostream& log_warning_f(const char*, int)
{
  static std::ostream s(nullptr);
  return s;
}
