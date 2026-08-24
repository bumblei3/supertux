// Minimal stub for engine symbols pulled in transitively by
// supertux/direction.cpp (util/log.hpp). gettext and Color are linked for
// real (util/gettext.cpp + video/color.cpp + tinygettext), only the logger
// global state is stubbed — same pattern as reader_test_stub.cpp.
#include <string>

// --- util/log.hpp symbols ---
int g_log_level = 0;
void log_warning_f(char const*, int) {}
