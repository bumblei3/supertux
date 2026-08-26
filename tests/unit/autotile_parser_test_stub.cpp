//  SuperTux
//  Copyright (C) 2026 SuperTux contributors
//  License: GPL-2+

// Stub for AutotileParserTest: gameconfig.cpp is linked for real (the parser
// reads g_config->developer_mode), so its transitive heavy deps that the test
// never exercises are stubbed here. util/log.cpp is ALSO linked — do NOT
// redefine g_log_level / log_*_f here (ASan ODR clash).

#include "video/video_system.hpp"
#include "video/viewport.hpp"
#include "editor/editor.hpp"
#include "physfs/ifile_stream.hpp"
#include "physfs/ofile_stream.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "physfs/ofile_streambuf.hpp"

#include <physfs.h>

#include <curl/curl.h>
#include <cstdlib>
#include <cstring>

char* curl_easy_escape(CURL*, const char* s, int) { return strdup(s); }
void curl_free(void* p) { free(p); }

// Real libphysfs is linked; initialize it and mount "/" so absolute paths
// to temp files resolve natively. No shim needed.
static const bool s_physfs_ready = [] {
  if (PHYSFS_init(nullptr) == 0) return false;
  return PHYSFS_mount("/", nullptr, 0) != 0;
}();

bool Editor::is_active() { return false; }
std::string VideoSystem::get_video_string(VideoSystem::Enum) { return ""; }
VideoSystem::Enum VideoSystem::get_video_system(const std::string&) { return VideoSystem::VIDEO_SDL; }
void Viewport::force_full_viewport(bool, bool) {}

// console/physfs-stream/UID-stream symbols referenced by util/log.cpp and
// util/writer.cpp (both linked for real here). Inert at LOG_NONE.
#include "supertux/console.hpp"
#include "physfs/ifile_stream.hpp"
#include "physfs/ofile_stream.hpp"
#include "util/uid.hpp"

bool Console::hasFocus() const { return false; }
void Console::open() {}
ConsoleStreamBuffer ConsoleBuffer::s_outputBuffer;
std::ostream ConsoleBuffer::output(&ConsoleBuffer::s_outputBuffer);
void ConsoleBuffer::flush(ConsoleStreamBuffer&) {}
Console::~Console() = default;
ssq::VM::~VM() = default;
ssq::Object::~Object() = default;

std::ostream& operator<<(std::ostream& os, const UID& uid) { return os << uid.get_value(); }


// Control enum helpers + editor gate + physfs file streams referenced by
// gameconfig.cpp / tile_set_parser.cpp. The Control conversions are real
// implementations inlined here to avoid linking the input-manager stack.
#include "interface/control.hpp"
#include "editor/editor.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "physfs/ofile_streambuf.hpp"
const char* g_control_names[] = { nullptr };
// Real implementations from control/controller.cpp (small, header-free logic):
std::string Control_to_string(Control control) { return g_control_names[static_cast<int>(control)]; }
std::optional<Control> Control_from_string(const std::string& text)
{
  for (int i = 0; g_control_names[i] != nullptr; ++i) {
    if (text == g_control_names[i]) return static_cast<Control>(i);
  }
  return std::nullopt;
}
std::ostream& operator<<(std::ostream& os, Control control) { return os << Control_to_string(control); }

int Viewport::get_screen_width() const { return 0; }
int Viewport::get_screen_height() const { return 0; }

#include "physfs/physfs_sdl.hpp"
SDL_IOStream* get_physfs_SDLRWops(const std::string&) { return nullptr; }
SDL_IOStream* get_writable_physfs_SDLRWops(const std::string&) { return nullptr; }
