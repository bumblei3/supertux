// Stubs for engine symbols pulled in transitively by the Reader source files
// (reader_document.cpp -> physfs/ifile_stream.hpp, util/file_system.hpp,
// util/log.hpp). Linking the real logging/filesystem/PhysFS stack would drag
// in Squirrel and the full engine, so we provide minimal placeholders to keep
// ReaderTest a self-contained logic test focused on the parser.
#include <string>

// --- util/log.hpp symbols ---
int g_log_level = 0;
void log_warning_f(char const*, int) {}
void log_info_f(char const*, int) {}
void log_debug_f(char const*, int, bool) {}
void log_fatal_f(char const*, int) {}

// --- util/file_system.hpp symbols ---
namespace FileSystem {
  std::string normalize(std::string const& filename) { return filename; }
  std::string dirname(std::string const& filename)
  {
    auto pos = filename.find_last_of('/');
    if (pos == std::string::npos)
      return std::string();
    return filename.substr(0, pos);
  }
  bool exists(std::string const& /*filename*/) { return false; }
}

// --- physfs/ifile_stream.hpp ---
// Minimal stand-in for IFileStream. reader_document.cpp constructs one and
// uses it as a std::istream to parse from a buffer we don't actually read in
// the unit test, so we provide a trivial stringbuf-backed istream.
#include <istream>
#include <memory>
#include <sstream>
#include <streambuf>

class IFileStream final : public std::istream
{
protected:
  std::unique_ptr<std::streambuf> sb;

public:
  IFileStream(const std::string& filename);
  IFileStream(const IFileStream&) = delete;
  IFileStream& operator=(const IFileStream&) = delete;
};

IFileStream::IFileStream(const std::string& /*filename*/)
  : std::istream(new std::stringbuf())
{
  sb.reset(const_cast<std::streambuf*>(rdbuf()));
}
