//  SuperTux
//  Copyright (C) 2026 SuperTux contributors
//  License: GPL-2+

// Stub for AutotileParserTest: gameconfig.cpp is linked for real (the parser
// reads g_config->developer_mode), so its transitive heavy deps that the test
// never exercises are stubbed here. util/log.cpp is ALSO linked — do NOT
// redefine g_log_level / log_*_f here (ASan ODR clash).

#include "control/joystick_config.hpp"
#include "control/keyboard_config.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

#include <curl/curl.h>
#include <cstdlib>
#include <cstring>

JoystickConfig::JoystickConfig() {}
void JoystickConfig::read(const ReaderMapping&) {}
void JoystickConfig::write(Writer&) {}

KeyboardConfig::KeyboardConfig() {}
void KeyboardConfig::read(const ReaderMapping&) {}
void KeyboardConfig::write(Writer&) {}

char* curl_easy_escape(CURL*, const char* s, int) { return strdup(s); }
void curl_free(void* p) { free(p); }

extern "C" {
// Working stdio-backed PHYSFS shims: absolute paths work as-is, no mount needed.
static FILE* to_file(void* p) { return static_cast<FILE*>(p); }
void* PHYSFS_openRead(const char* path) { return fopen(path, "rb"); }
void* PHYSFS_openWrite(const char* path) { return fopen(path, "wb"); }
int PHYSFS_close(void* f) { return fclose(to_file(f)); }
long long PHYSFS_fileLength(void* f) {
  long long cur = ftell(to_file(f));
  fseek(to_file(f), 0, SEEK_END);
  long long size = ftell(to_file(f));
  fseek(to_file(f), (long)cur, SEEK_SET);
  return size;
}
int PHYSFS_readBytes(void* f, void* buf, unsigned int n) { return (int)fread(buf, 1, n, to_file(f)); }
int PHYSFS_writeBytes(void* f, const void* buf, unsigned int n) { return (int)fwrite(buf, 1, n, to_file(f)); }
int PHYSFS_eof(void* f) { return feof(to_file(f)) ? 1 : 0; }
long long PHYSFS_tell(void* f) { return ftell(to_file(f)); }
int PHYSFS_seek(void* f, unsigned long long pos) { return fseek(to_file(f), (long)pos, SEEK_SET); }
int PHYSFS_delete(const char*) { return 0; }
int PHYSFS_init(const char*) { return 1; }
int PHYSFS_mount(const char*, const char*, int) { return 1; }
}
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

#include "physfs/util.hpp"
namespace physfsutil { const char* get_last_error() { return "stub"; } }
