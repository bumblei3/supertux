// Additional stubs for ReaderGetLayerTest: reader_test_stub.cpp provides the
// log/FileSystem/IFileStream symbols, but util/reader.cpp also calls
// Editor::is_active() (must be false so the LAYER_GUI clamp is active) and
// PHYSFS_getRealDir — which segfaults when PHYSFS is not initialised, so it
// must be stubbed out entirely (returning nullptr = "no search path").
//
// Editor::is_active() is a static member requiring a complete class; the
// editor.hpp include in this TU pulls squirrel headers but compiles fine
// with the simplesquirrel INCLUDES from CMakeLists.
#include <string>
#include <physfs.h>

#include "editor/editor.hpp"

bool Editor::is_active() { return false; }

const char* PHYSFS_getRealDir(const char*)
{
  return nullptr;
}
