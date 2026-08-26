//  SuperTux
//  Copyright (C) 2026 SuperTux contributors
//  License: GPL-2+

// Variant of writer_test_stub for targets that LINK util/log.cpp (which
// defines the log_*_f functions and g_log_* globals itself — redefining
// them here trips ASan ODR checks). Provides only the console/physfs-stream/
// UID symbols that log.cpp and writer.cpp reference.

#include <ostream>
#include <string>

#include "physfs/ofile_stream.hpp"
#include "physfs/ifile_stream.hpp"
#include "util/uid.hpp"

OFileStream::OFileStream(const std::string&) : std::ostream(nullptr) {}
// util/file_system.cpp is NOT linked into this target (it pulls physfs/curl);
// reader_document.cpp's get_directory() needs dirname only.
namespace FileSystem {
std::string dirname(const std::string& filename) { return filename; }
}
IFileStream::IFileStream(const std::string&) : std::istream(nullptr) {}
std::ostream& operator<<(std::ostream& os, const UID& uid) { return os << uid.get_value(); }

#include "supertux/console.hpp"

bool Console::hasFocus() const { return false; }
void Console::open() {}
ConsoleStreamBuffer ConsoleBuffer::s_outputBuffer;
std::ostream ConsoleBuffer::output(&ConsoleBuffer::s_outputBuffer);
void ConsoleBuffer::flush(ConsoleStreamBuffer&) {}
Console::~Console() = default;
ssq::VM::~VM() = default;
ssq::Object::~Object() = default;
