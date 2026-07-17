add_subdirectory(unit)
# E2E smoke tests run the real supertux2 binary headless (xvfb) to catch
# startup/fullscreen regressions that the unit tests (dummy SDL driver) miss.
add_subdirectory(e2e)
