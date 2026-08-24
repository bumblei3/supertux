// Minimal stubs for engine symbols pulled in transitively by the TileSet /
// TileSetParser / Surface / TextureManager sources. The globals (g_config,
// g_game_time) and most log functions are already provided by
// tile_test_stub.cpp, which is linked into this target too.
#include <string>

// --- util/log.hpp symbols (not in tile_test_stub.cpp) ---
void log_debug_f(char const*, int, bool) {}
void log_fatal_f(char const*, int) {}
