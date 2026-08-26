// Minimal stubs for math/anchor_point.cpp linkage (g_log_level, log_warning_f).
// Same symbols as anchor_point_test_stub.cpp; ODR-safe when both are linked.

#include "util/log.hpp"
#include "util/gettext.hpp"

#include <memory>
#include <ostream>

LogLevel g_log_level = LOG_NONE;
bool g_log_tinygettext = false;

std::unique_ptr<tinygettext::DictionaryManager> g_dictionary_manager;

std::ostream& log_warning_f(const char*, int)
{
  static std::ostream s(nullptr);
  return s;
}
