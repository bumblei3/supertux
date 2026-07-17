// Minimal stubs for symbols that util/uid_generator.cpp pulls in from the
// engine logging layer (util/log.cpp -> supertux/console.hpp -> squirrel).
// Linking the full logging stack into a unit test drags in Squirrel, so we
// provide tiny placeholders instead. This keeps UIDTest a pure-logic test.
//
// UID::operator<< is declared in uid.hpp but never defined anywhere in the
// codebase, so we define it here (it is only used by GTest's value printer).
#include <ostream>

#include "util/uid.hpp"

int g_log_level = 0;

void log_warning_f(char const*, int) {}

std::ostream& operator<<(std::ostream& os, const UID& uid)
{
  return os << uid.get_value();
}
