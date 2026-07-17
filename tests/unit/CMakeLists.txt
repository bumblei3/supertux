# Will eventually handle game tests here too
include(CTest)

find_package(GTest REQUIRED)
find_package(SDL3 REQUIRED)

function(make_unit_test test_name)
  cmake_parse_arguments(PARSE_ARGV 1 mtargs
    "NO_PREPEND_SRC" "" "SOURCE;EXTERNAL;LIBRARIES;DEFINITIONS;INCLUDES")
  # EXTERNAL is just more readable for external source files we use
  if (NOT mtargs_NO_PREPEND_SRC)
    list(TRANSFORM mtargs_EXTERNAL PREPEND ${SUPERTUX_SOURCE_DIR}/src/)
  endif()
  add_executable(${test_name} ${mtargs_SOURCE} ${mtargs_EXTERNAL})
  target_compile_features(${test_name} PRIVATE cxx_std_17)
  target_include_directories(${test_name} PUBLIC ${SUPERTUX_SOURCE_DIR}/src ${mtargs_INCLUDES})
  if (mtargs_DEFINITIONS)
    target_compile_definitions(${test_name} PUBLIC ${mtargs_DEFINITIONS})
  endif()
  if (mtargs_LIBRARIES)
    target_link_libraries(${test_name} PUBLIC ${mtargs_LIBRARIES})
  endif()
  set(all_test_targets "${all_test_targets};${test_name}" CACHE INTERNAL "")
  add_test(NAME ${test_name}
    COMMAND ${test_name})
endfunction(make_unit_test)

make_unit_test(RectTest SOURCE rect_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main SDL3::SDL3
  DEFINITIONS GLM_ENABLE_EXPERIMENTAL
  EXTERNAL math/rect.cpp)

make_unit_test(MD5Test SOURCE md5_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main
  EXTERNAL addon/md5.cpp)

# Lightweight pure-logic unit tests. These only need their single source file
# (no engine / tinygettext / SDL renderer linkage) and were previously left
# unwired in the tree.
make_unit_test(SizeTest SOURCE size_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main
  DEFINITIONS GLM_ENABLE_EXPERIMENTAL
  EXTERNAL math/size.cpp)

make_unit_test(SizefTest SOURCE sizef_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main
  DEFINITIONS GLM_ENABLE_EXPERIMENTAL
  EXTERNAL math/sizef.cpp)

make_unit_test(RectfTest SOURCE rectf_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main
  DEFINITIONS GLM_ENABLE_EXPERIMENTAL
  EXTERNAL math/rectf.cpp)

make_unit_test(RandomTest SOURCE random_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main
  EXTERNAL math/random.cpp)

make_unit_test(StringUtilTest SOURCE string_util_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main
  EXTERNAL util/string_util.cpp)

make_unit_test(VersionTest SOURCE version_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main
  INCLUDES ${CMAKE_BINARY_DIR})

# UID generator: pure logic, single source file.
# We don't link util/log.cpp (it pulls in the Squirrel-based console layer);
# instead uid_test_stub.cpp provides the few logging symbols uid_generator.cpp
# references, keeping this a self-contained logic test.
make_unit_test(UIDTest SOURCE uid_test.cpp uid_test_stub.cpp
  LIBRARIES GTest::gtest GTest::gtest_main
  EXTERNAL util/uid_generator.cpp)

# NOTE: ReaderTest / ObjectOptionTest / FileSystemTest / IFileStreamTest /
# DynamicScopedTest are intentionally NOT wired here: they require the full
# engine stack (tinygettext, sexp-cpp, Squirrel, PhysFS) or are explicitly
# marked DISABLED in the source. See commit history for the rationale.

# SDL3 API / fullscreen regression tests. These pin the SDL2->SDL3 API
# contract (SDL_SetWindowFullscreen returns bool; SDL_RectsEqual rename) and
# the fixed fullscreen startup path so a future SDL break fails here instead
# of only showing up as "SuperTux exits instantly on --fullscreen".
make_unit_test(SDL3ApiTest SOURCE sdl3_api_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main SDL3::SDL3)

make_unit_test(SDLVideoModeTest SOURCE sdl_video_mode_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main SDL3::SDL3)

make_unit_test(AATriangleTest SOURCE aatriangle_test.cpp
  EXTERNAL math/aatriangle.cpp
  LIBRARIES SDL3::SDL3 glm DEFINITIONS GLM_ENABLE_EXPERIMENTAL)

make_unit_test(CollisionTest SOURCE collision_test.cpp
  EXTERNAL math/rectf.cpp
  LIBRARIES SDL3::SDL3 glm DEFINITIONS GLM_ENABLE_EXPERIMENTAL)

message("ALL TESTS: ${all_test_targets}")

add_custom_target(tests DEPENDS ${all_test_targets})
add_custom_command(TARGET tests POST_BUILD COMMAND ctest -C $<CONFIGURATION> --output-on-failure)
