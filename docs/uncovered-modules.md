# Known Uncovered Modules (engine-woven)

This document lists source modules that are intentionally NOT covered by
unit tests, and explains WHY. It exists so the next developer does not
waste time trying to write isolated unit tests for code that is
deliberately engine-woven.

Measured baseline: `build-cov` gcovr run, 2026-08-27 — 54.1% overall
line coverage (2822/5214). The modules below are all <5% line coverage.

## Why these are not unit-tested

Most of them pull in one or more of:

- **SDL / SDL3** (`src/video/*`) — windowing, surfaces, GL context.
  Needs a real `SDL_CreateWindow` + GL context to instantiate.
- **PHYSFS** (`src/physfs/*`) — virtual file system. Needs
  `PHYSFS_init` + a mounted search path to do anything.
- **Squirrel VM** (`src/supertux/sector.hpp` → GameObjectManager →
  simplesquirrel) — the scripting runtime.

An isolated unit test for any of these would have to stub/mock the whole
SDL/GL/PHYSFS/Squirrel stack, which:

1. Does not actually exercise the real code path (the mock replaces the
   interesting part), and
2. Is fragile — breaks on every upstream refactor of the video/physfs
   layer.

The correct guard for this code is the **E2E smoke suite**
(`tests/e2e/smoke_*.sh`), which boots the real binary headless under
xvfb and asserts it does not crash through the boot / level-load /
render-loop / gameplay paths. That is where the texture_manager
use-after-free fix and the PlayerStatusHUD null-deref regression were
actually caught.

## The list

### Video stack (`src/video/*`) — all 0%
- `texture_manager.cpp` (519 lines) — loads images, uploads GL textures.
  Contains the SDL_CreateSurface UAF fix. Only E2E-reachable.
- `surface.cpp`, `sdl_surface.cpp` — SDL surface wrappers.
- `texture.cpp`, `sampler.cpp`, `canvas.cpp`, `drawing_context.cpp`,
  `surface_batch.cpp` — drawing / GL state.
- `sdl_surface_ptr.hpp` — RAII SDL surface holder.

> Note: `surface.cpp` has zero SDL/GL/PHYSFS includes in its header, so
> it *could* be linked in isolation with a stub — but it still depends on
> the `Surface` class which is constructed from an SDL surface, so the
> payoff is small. Left unwired for now.

### PHYSFS (`src/physfs/*`) — 0–4%
- `ifile_stream.cpp`, `ofile_stream.cpp`, `ofile_streambuf.cpp` — stream
  wrappers over PHYSFS file handles. Need `PHYSFS_init` + mounted path.
- `physfs_sdl.cpp` — SDL RWops over PHYSFS. SDL + PHYSFS.
- `util.cpp` — `realpath`, `is_directory`, `get_last_error`. Needs
  PHYSFS_init. (`physfsutil::realpath` wraps `FileSystem::normalize`,
  which is pure — but the other functions need a live VFS.)

### Game logic (`src/supertux/*`) — 0%
- `sector.hpp` — the live game world. Pulls GameObjectManager +
  SquirrelEnvironment + video/gl.hpp transitively. Engine-woven by
  definition; cannot be instantiated without a running game session.

### Utility (`src/util/*`) — 0–4%
- `uid.cpp` — only `operator<<(UID)` + `std::hash<UID>` specialization.
  The `UID` wrapper class itself is covered by `uid_test.cpp`; the
  free-function/extension points here are trivial and link-time collapsed.
  Not worth a dedicated test.

## Modules that ARE covered (for contrast)

The following are fully unit-tested and should stay that way — the
coverage gate (`scripts/coverage_gate.py`) enforces them:

- `math/*` (rect, rectf, size, sizef, aatriangle, random, easing,
  vector, util) — 100%
- `supertux/timer.cpp`, `sequence.cpp`, `physic.hpp`, `autotile*.{cpp,hpp}`,
  `tile*.{cpp,hpp}` — 95–100%
- `util/*` (string_util, reader_*, file_system, fade_helper,
  colorspace_oklab, file_watcher, uid, unique_name) — 95–100%

## How to add coverage here (if ever needed)

If a regression in one of these modules is NOT caught by E2E, prefer
extending the E2E smoke suite (e.g. a new `smoke_*.sh` that drives a
specific path) over writing a mocked unit test. Only write an isolated
unit test if the module can be linked with a *small, stable* stub (see
`tests/unit/physic_test_stub.cpp` and `file_system_test_stub.cpp` for
the established pattern) — and even then, keep the stub minimal and
document why the engine dependency is safe to stub.
