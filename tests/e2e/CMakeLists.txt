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
