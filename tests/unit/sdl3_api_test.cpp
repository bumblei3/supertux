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

// Regression tests for the SDL2 -> SDL3 API breaks that previously crashed or
// misbehaved in SuperTux's video code. These run against the real SDL3 library
// (HEADLESS via the dummy driver) so they fail loudly if SDL3's semantics
// change again.

#include <gtest/gtest.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "math/rect.hpp"

// Ensure we exercise SDL3, not a compatibility shim.
#if !defined(SDL_VERSION_ATLEAST)
#  error "SDL_VERSION_ATLEAST macro missing"
#endif
#if !SDL_VERSION_ATLEAST(3, 0, 0)
#  error "These tests require SDL3"
#endif

namespace {

// Run all SDL-using tests against the headless dummy driver so they work in CI
// and on machines without a display server.
class SDL3ApiTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Use the dummy driver by default (headless, no display needed). But if
    // SDL_VIDEODRIVER is already set (e.g. the CI runs under xvfb with
    // SDL_VIDEODRIVER=x11), honor it so the test exercises the real driver and
    // can catch compositor-specific bugs.
    const char* driver = SDL_getenv("SDL_VIDEODRIVER");
    if (driver == nullptr || driver[0] == '\0')
    {
      SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
    }
    ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO));
  }

  void TearDown() override
  {
    SDL_Quit();
  }
};

} // namespace

// SuperTux historically called SDL_SetWindowFullscreen(...) and checked the
// result with `!= 0`. In SDL2 that was correct (0 == success, int return). In
// SDL3 the function returns a bool with true == success. A `!= 0` check on the
// bool return is inverted and would log a false "failed to switch to fullscreen"
// warning on every successful switch. This test pins the SDL3 contract: the
// return type is bool and true means success.
TEST_F(SDL3ApiTest, SetWindowFullscreenReturnsBool)
{
  SDL_Window* window = SDL_CreateWindow("tux", 640, 480, 0);
  ASSERT_NE(window, nullptr);

  // The call must succeed and return true (bool success) under the dummy driver.
  const bool result = SDL_SetWindowFullscreen(window, true);
  EXPECT_TRUE(result) << "SDL_SetWindowFullscreen(true) must return true on success (SDL3 bool contract)";

  // Toggling back off must also report success.
  EXPECT_TRUE(SDL_SetWindowFullscreen(window, false));

  SDL_DestroyWindow(window);
}

// SuperTux's Rect::to_sdl() produces an SDL_Rect that is compared with
// SDL_RectsEqual. In SDL3 the function was renamed from SDL_RectEquals to
// SDL_RectsEqual; the old name only survives as a deprecated alias via
// SDL_oldnames.h. This test pins the SDL3 name so a future SDL removal of the
// alias breaks the build here instead of deep in video code.
TEST_F(SDL3ApiTest, RectEqualsUsesSDL3Name)
{
  const SDL_Rect a{50, 50, 50, 50};
  const SDL_Rect b{50, 50, 50, 50};
  EXPECT_TRUE(SDL_RectsEqual(&a, &b));

  const SDL_Rect c{0, 0, 10, 10};
  EXPECT_FALSE(SDL_RectsEqual(&a, &c));

  // Make sure SuperTux's Rect <-> SDL_Rect conversion stays consistent with the
  // SDL3 comparison helper.
  const SDL_Rect from_tux = Rect(50, 50, 100, 100).to_sdl();
  EXPECT_TRUE(SDL_RectsEqual(&from_tux, &a));
}

/* EOF */
