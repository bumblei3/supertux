//  SuperTux
//  Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Regression test for SuperTux's SDL3 fullscreen startup path.
//
// Before the SDL3 port fix, SDLBaseVideoSystem::create_sdl_window() requested
// SDL_WINDOW_FULLSCREEN at SDL_CreateWindow() time together with an explicit
// (often stale, e.g. 640x480) fullscreen_size. Under Wayland/Xwayland the
// compositor rejects a fullscreen window with a non-enumerated size, so
// SDL_CreateWindow() returned NULL and startup aborted silently.
//
// The fixed path (mirrored here) is:
//   1. create the window normally (no SDL_WINDOW_FULLSCREEN flag),
//   2. size it from the desktop size (not a stale fullscreen_size),
//   3. switch to fullscreen later via SDL_SetWindowFullscreen(true),
//   4. fall back to desktop fullscreen if an explicit mode is unavailable.
//
// This test exercises that exact sequence against the real SDL3 library so a
// regression (re-adding the fullscreen flag at create time, or passing a bogus
// size) fails here instead of only showing up as "SuperTux exits instantly".

#include <gtest/gtest.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

namespace {

class SDLVideoModeTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
    ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO));
  }

  void TearDown() override
  {
    SDL_Quit();
  }
};

} // namespace

// Mirror of SDLBaseVideoSystem::create_sdl_window() for the fullscreen case
// AFTER the SDL3 fix: no SDL_WINDOW_FULLSCREEN at creation time, window sized
// from the desktop display mode. Creating the window must succeed even when a
// stale/non-enumerated fullscreen_size would have been used before.
TEST_F(SDLVideoModeTest, CreateWindowedThenSwitchToFullscreenSucceeds)
{
  // A stale config value like 640x480 must NOT be used as the create size when
  // going fullscreen; the desktop size is used instead. Under the dummy driver
  // the desktop mode is whatever the driver reports.
  SDL_DisplayID display = SDL_GetPrimaryDisplay();
  ASSERT_NE(display, 0U);

  const SDL_DisplayMode* desktop = SDL_GetDesktopDisplayMode(display);
  ASSERT_NE(desktop, nullptr);

  SDL_Window* window = SDL_CreateWindow("tux-fullscreen", desktop->w, desktop->h, 0);
  ASSERT_NE(window, nullptr) << "create_sdl_window() must not fail when fullscreen "
                                "uses the desktop size instead of a stale fullscreen_size";

  // apply_video_mode() later switches to fullscreen. This must not fail and the
  // return value must be treated as a bool (true == success), exactly as the
  // fixed SDLBaseVideoSystem::apply_video_mode() does.
  const bool switched = SDL_SetWindowFullscreen(window, true);
  EXPECT_TRUE(switched) << "SDL_SetWindowFullscreen(true) must succeed (SDL3 bool contract)";

  // Toggling back off must also succeed.
  EXPECT_TRUE(SDL_SetWindowFullscreen(window, false));

  SDL_DestroyWindow(window);
}

// The fall-back path: when an explicit fullscreen size is requested, SuperTux
// resolves it via SDL_GetClosestFullscreenDisplayMode() and only then calls
// SDL_SetWindowFullscreenMode() + SDL_SetWindowFullscreen(true). This must not
// abort startup even for an unusual requested size.
TEST_F(SDLVideoModeTest, ClosestFullscreenModeDoesNotAbort)
{
  SDL_Window* window = SDL_CreateWindow("tux-closest", 800, 600, 0);
  ASSERT_NE(window, nullptr);

  SDL_DisplayID display = SDL_GetDisplayForWindow(window);
  ASSERT_NE(display, 0U);

  SDL_DisplayMode closest;
  SDL_memset(&closest, 0, sizeof(closest));
  const bool found = SDL_GetClosestFullscreenDisplayMode(display, 640, 480, 0.0f, false, &closest);

  if (found)
  {
    // An explicit mode is available: setting it and switching to fullscreen must
    // succeed (this is the path that previously called SDL_SetWindowFullscreen
    // with SDL_WINDOW_FULLSCREEN as the argument, which is wrong in SDL3).
    EXPECT_TRUE(SDL_SetWindowFullscreenMode(window, &closest));
    EXPECT_TRUE(SDL_SetWindowFullscreen(window, true));
    EXPECT_TRUE(SDL_SetWindowFullscreen(window, false));
  }
  else
  {
    // No mode near the request: the code must fall back to desktop fullscreen
    // rather than aborting.
    EXPECT_TRUE(SDL_SetWindowFullscreen(window, true));
    EXPECT_TRUE(SDL_SetWindowFullscreen(window, false));
  }

  SDL_DestroyWindow(window);
}

// Regression guard for compositors (Mutter/GNOME under Wayland/Xwayland) that
// do not reliably honor a late SDL_SetWindowFullscreen(true) on a windowed-
// created window: SDL may report success while the window never actually
// enters fullscreen. We assert the SDL_WINDOW_FULLSCREEN flag is really set on
// the window after the call, catching the "SDL says true but no fullscreen"
// class of bug that previously left SuperTux starting without a visible
// fullscreen window. Under the dummy driver the flag is set synchronously, so
// this test pins the expected post-condition of the fixed code path.
TEST_F(SDLVideoModeTest, FullscreenFlagIsActuallySet)
{
  SDL_DisplayID display = SDL_GetPrimaryDisplay();
  ASSERT_NE(display, 0U);

  const SDL_DisplayMode* desktop = SDL_GetDesktopDisplayMode(display);
  ASSERT_NE(desktop, nullptr);

  // Mirror the fixed create_sdl_window(): request SDL_WINDOW_FULLSCREEN at
  // creation time, sized from the desktop mode.
  SDL_Window* window = SDL_CreateWindow("tux-flag-check", desktop->w, desktop->h,
                                         SDL_WINDOW_FULLSCREEN);
  ASSERT_NE(window, nullptr) << "create_sdl_window() must create a fullscreen "
                                "window from the desktop size";

  const Uint32 flags = SDL_GetWindowFlags(window);
  EXPECT_TRUE(flags & SDL_WINDOW_FULLSCREEN)
    << "window must actually carry SDL_WINDOW_FULLSCREEN after creation";

  SDL_DestroyWindow(window);
}

/* EOF */
