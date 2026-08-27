// Minimal stubs for symbols that util/uid_generator.cpp pulls in from the
// engine logging layer (util/log.cpp -> supertux/console.hpp -> squirrel).
// Linking the full logging stack into a unit test drags in Squirrel, so we
// provide tiny placeholders instead. This keeps UIDTest a pure-logic test.
// UID::operator<< is defined in util/uid.cpp, so we do not redefine it here.

#include <ostream>

#include "util/uid.hpp"

int g_log_level = 0;

void log_warning_f(char const*, int) {}
