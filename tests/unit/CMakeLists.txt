# Will eventually handle game tests here too
include(CTest)

find_package(GTest REQUIRED)
find_package(SDL3 REQUIRED)

function(make_unit_test test_name)
  cmake_parse_arguments(PARSE_ARGV 1 mtargs
    "NO_PREPEND_SRC;GTEST;SDL2" "" "SOURCE;EXTERNAL;LIBRARIES;DEFINITIONS;INCLUDES")
  # EXTERNAL is just more readable for external source files we use
  if (NOT mtargs_NO_PREPEND_SRC)
    list(TRANSFORM mtargs_EXTERNAL PREPEND ${SUPERTUX_SOURCE_DIR}/src/)
  endif()
  add_executable(${test_name} ${mtargs_SOURCE} ${mtargs_EXTERNAL})
  target_compile_features(${test_name} PRIVATE cxx_std_17)
  target_compile_definitions(${test_name} PUBLIC GLM_ENABLE_EXPERIMENTAL)
  target_include_directories(${test_name} PUBLIC ${SUPERTUX_SOURCE_DIR}/src ${mtargs_INCLUDES})
  if (mtargs_DEFINITIONS)
    target_compile_definitions(${test_name} PUBLIC ${mtargs_DEFINITIONS})
  endif()
  set(_libs "${mtargs_LIBRARIES}")
  if (mtargs_SDL2)
    list(APPEND _libs SDL2::SDL2)
  endif()
  if (mtargs_GTEST)
    target_link_libraries(${test_name} PUBLIC GTest::gtest GTest::gtest_main ${_libs})
  elseif (_libs)
    target_link_libraries(${test_name} PUBLIC ${_libs})
  endif()
  set(all_test_targets "${all_test_targets};${test_name}" PARENT_SCOPE)
  add_test(NAME ${test_name}
    COMMAND $<TARGET_FILE:${test_name}>)
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

# Reader: parses the lisp-style config format. Pulls in reader_document/
# reader_mapping/reader_collection plus the tinygettext and sexp-cpp libraries
# (both already built as static libs). The engine logging/filesystem layer is
# stubbed out by reader_test_stub.cpp to keep this a self-contained logic test.
make_unit_test(ReaderTest SOURCE reader_test.cpp reader_test_stub.cpp
  LIBRARIES GTest::gtest GTest::gtest_main
            "$<LINK_GROUP:RESCAN,SDL3::SDL3;tinygettext;sexp>"
  EXTERNAL util/reader_document.cpp util/reader_mapping.cpp
            util/reader_collection.cpp util/reader_iterator.cpp
            util/reader_object.cpp util/gettext.cpp
  INCLUDES ${SUPERTUX_SOURCE_DIR}/external/sexp-cpp/include
            ${SUPERTUX_SOURCE_DIR}/external/tinygettext/include)

# SDL3 API / fullscreen regression tests. These pin the SDL2->SDL3 API
# contract (SDL_SetWindowFullscreen returns bool; SDL_RectsEqual rename) and
# the fixed fullscreen startup path so a future SDL break fails here instead
# of only showing up as "SuperTux exits instantly on --fullscreen".
make_unit_test(SDL3ApiTest SOURCE sdl3_api_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main SDL3::SDL3)

make_unit_test(SDLVideoModeTest SOURCE sdl_video_mode_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main SDL3::SDL3)

make_unit_test(AATriangleTest SOURCE aatriangle_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main SDL3::SDL3 glm
  DEFINITIONS GLM_ENABLE_EXPERIMENTAL
  EXTERNAL math/aatriangle.cpp)

make_unit_test(CollisionTest SOURCE collision_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main SDL3::SDL3 glm
  DEFINITIONS GLM_ENABLE_EXPERIMENTAL
  EXTERNAL math/rectf.cpp)

# CollisionModuleTest: regression coverage for the previously-untested
# collision/ module (Constraints, set_rectangle_rectangle_constraints,
# line_intersects_line, intersects_line, rectangle_aatriangle slope math).
make_unit_test(CollisionModuleTest SOURCE collision_module_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main SDL3::SDL3 glm
  DEFINITIONS GLM_ENABLE_EXPERIMENTAL
  EXTERNAL collision/collision.cpp math/rectf.cpp math/aatriangle.cpp)

# EasingTest: covers math/easing.cpp (the elastic easing overshoots by design).
make_unit_test(EasingTest SOURCE easing_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main SDL3::SDL3 glm
  DEFINITIONS GLM_ENABLE_EXPERIMENTAL
  EXTERNAL math/easing.cpp util/string_util.cpp)

# Dependency-free coverage for string_util (st_assert harness, no gtest/SDL).
# Covers the replace_all empty-needle infinite-loop fix.
make_unit_test(StringUtilStTest SOURCE string_util_st_test.cpp
  EXTERNAL util/string_util.cpp)

# TileReplacementTest: regression coverage for the editor tile-replacement
# degenerate/division-by-zero guard (issue #3810).
make_unit_test(TileReplacementTest SOURCE tile_replacement_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main SDL3::SDL3 glm
  EXTERNAL editor/tile_replacement.hpp)

message("ALL TESTS: ${all_test_targets}")

add_custom_target(tests)
add_dependencies(tests ${all_test_targets})
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(check ${all_test_targets})
