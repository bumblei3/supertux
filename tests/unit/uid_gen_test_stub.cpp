// Minimal stub for engine symbols pulled in transitively by
// util/uid_generator.cpp (util/log.hpp). uid.cpp itself defines
// operator<<(ostream&, UID const&), so this stub must NOT provide it.
#include <string>

// --- util/log.hpp symbols ---
int g_log_level = 0;

void log_warning_f(char const*, int) {}
