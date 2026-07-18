# E2E smoke tests for SuperTux.
#
# These run the *real* supertux2 binary headless under a virtual X display
# (xvfb) so that compositor-specific behavior (e.g. Wayland/Xwayland
# fullscreen, fixed in 6c30d803c) is exercised — something the unit tests
# (which use the headless dummy SDL driver) cannot catch.

find_program(XVFB_RUN_BIN NAMES xvfb-run)
if(NOT XVFB_RUN_BIN)
  message(STATUS "E2E tests disabled: xvfb-run not found")
  return()
endif()

# The supertux2 binary is built at the top of the build tree.
set(SUPERTUX_BIN "${CMAKE_BINARY_DIR}/supertux2")
if(NOT EXISTS "${SUPERTUX_BIN}")
  message(STATUS "E2E tests disabled: supertux2 binary not built "
                 "(${SUPERTUX_BIN})")
  return()
endif()

add_test(
  NAME SmokeFullscreen
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/smoke_fullscreen.sh" "${SUPERTUX_BIN}"
)

# Render-output smoke test: boots the real binary under xvfb and asserts
# the captured framebuffer is NOT black (catches "runs fine but renders
# nothing" regressions that a clean-exit/boot check cannot see). Skips
# gracefully if scrot/convert are unavailable.
add_test(
  NAME SmokeRenderPixels
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/smoke_render_pixels.sh" "${SUPERTUX_BIN}"
)

# Level-load smoke test: launches the real binary with .stl level files to
# exercise the level-parsing / GameSession-init path (which the
# menu/worldmap E2E never reaches). Does not assert on rendered pixels
# (a headless cutscene may not advance); only asserts loading does not
# crash. Skips gracefully if xvfb-run is unavailable.
add_test(
  NAME SmokeLevelLoad
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/smoke_level_load.sh" "${SUPERTUX_BIN}"
)
