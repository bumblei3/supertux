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
  EXTERNAL math/rect.cpp math/rectf.cpp)

make_unit_test(MD5Test SOURCE md5_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main
  EXTERNAL addon/md5.cpp)

make_unit_test(PhysicTest SOURCE physic_test.cpp physic_test_stub.cpp
  LIBRARIES GTest::gtest GTest::gtest_main
  DEFINITIONS GLM_ENABLE_EXPERIMENTAL
  EXTERNAL supertux/physic.cpp video/color.cpp
  INCLUDES ${SUPERTUX_SOURCE_DIR}/external/tinygettext/include
           ${SUPERTUX_SOURCE_DIR}/external/sexp-cpp/include
           ${SUPERTUX_SOURCE_DIR}/external/simplesquirrel/include
           ${SUPERTUX_SOURCE_DIR}/external/simplesquirrel/libs/squirrel/include
           ${CMAKE_BINARY_DIR})

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

# Pure path-manipulation helpers. file_system.cpp references PhysFS only in
# FileSystem::rename() (never called by this test), so a minimal link stub
# (file_system_test_stub.cpp) provides the PHYSFS symbols without pulling in
# the full library.
make_unit_test(FileSystemTest SOURCE file_system_test.cpp file_system_test_stub.cpp
  LIBRARIES GTest::gtest GTest::gtest_main
            "$<LINK_GROUP:RESCAN,SDL3::SDL3;tinygettext;sexp>"
  EXTERNAL util/file_system.cpp video/color.cpp
  INCLUDES ${SUPERTUX_SOURCE_DIR}/external/tinygettext/include
           ${SUPERTUX_SOURCE_DIR}/external/sexp-cpp/include
           ${CMAKE_BINARY_DIR})

make_unit_test(RectfTest SOURCE rectf_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main
  DEFINITIONS GLM_ENABLE_EXPERIMENTAL
  EXTERNAL math/rectf.cpp math/rect.cpp)

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

# Round-trip coverage for util/writer.hpp: write values into a std::ostream,
# parse them back with ReaderDocument::from_string, verify the save/load cycle.
# Logging/physfs stubbed (writer_test_stub.cpp); uses the ostream constructor
# so no file I/O is needed. Links sexp for the s-expression back-end.
make_unit_test(WriterTest SOURCE writer_test.cpp writer_test_stub.cpp
  LIBRARIES GTest::gtest GTest::gtest_main
            "$<LINK_GROUP:RESCAN,SDL3::SDL3;tinygettext;sexp>"
  EXTERNAL util/writer.cpp util/reader_document.cpp util/reader_mapping.cpp
            util/reader_collection.cpp util/reader_iterator.cpp
            util/reader_object.cpp util/gettext.cpp
  INCLUDES ${SUPERTUX_SOURCE_DIR}/external/sexp-cpp/include
            ${SUPERTUX_SOURCE_DIR}/external/tinygettext/include)

# sexp::Value round-trip (parser/serializer). Fully isolated: links only
# the sexp-cpp library (no engine / tinygettext / SDL), exercising the
# value types SuperTux's Writer/Reader ultimately produce and consume.
make_unit_test(SexpValueTest SOURCE sexp_value_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main sexp
  INCLUDES ${SUPERTUX_SOURCE_DIR}/external/sexp-cpp/include)

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

# Dependency-free coverage for DynamicScopedRef (header-only). Re-enables the
# test the original author had left disabled; the ST_ASSERT presence checks use
# static_cast<bool>() because DynamicScopedRef's operator bool() is explicit.
make_unit_test(DynamicScopedTest SOURCE dynamic_scoped_test.cpp)

# Dependency-free coverage for LineIterator (util/line_iterator.hpp): newline
# splitting, trailing-newline handling, interior empty lines, exhaustion.
make_unit_test(LineIteratorTest SOURCE line_iterator_test.cpp
  EXTERNAL util/line_iterator.cpp)

# Coverage for UTF8Iterator (util/utf8_iterator.hpp): 1-4 byte decoding,
# malformed-sequence handling, and the 4-byte off-by-one regression. Logging
# is stubbed (utf8_iterator_test_stub.cpp) to avoid the engine console layer.
make_unit_test(UTF8IteratorTest SOURCE utf8_iterator_test.cpp utf8_iterator_test_stub.cpp
  EXTERNAL util/utf8_iterator.cpp)

# Coverage for ColorOKLCh (util/colorspace_oklab.hpp): sRGB->OKLab LCh
# conversion against reference values, greyscale hue determinism, lightness
# monotonicity, and the modified-lightness estimate. Links video/color.cpp
# (needs SDL3 via color.hpp) with logging stubbed.
make_unit_test(ColorspaceOklabTest SOURCE colorspace_oklab_test.cpp colorspace_oklab_test_stub.cpp
  LIBRARIES SDL3::SDL3
  EXTERNAL util/colorspace_oklab.cpp video/color.cpp)

# Coverage for video/color.hpp string (de)serialization: deserialize_from_hex,
# deserialize_from_rgb, and the serialize/deserialize round-trips (valid
# parsing, out-of-range rejection, malformed-input handling). Logging stubbed.
make_unit_test(ColorParseTest SOURCE color_parse_test.cpp color_parse_test_stub.cpp
  LIBRARIES SDL3::SDL3
  EXTERNAL video/color.cpp)

# Coverage for collision/collision.hpp pure geometry (line_intersects_line,
# intersects_line, rectangle_aatriangle) not asserted by the gtest CollisionTest.
# collision.cpp pulls in math/aatriangle.hpp + supertux/physic.hpp (both
# header-only); logging stubbed (collision_geom_test_stub.cpp).
make_unit_test(CollisionGeomTest SOURCE collision_geom_test.cpp collision_geom_test_stub.cpp
  EXTERNAL collision/collision.cpp
  INCLUDES ${CMAKE_BINARY_DIR})

# Coverage for math/anchor_point.hpp: the 9 anchor positions over a known
# Rectf, placed-object offsets, quarter-offset centres, and string<->anchor
# round-trips. gettext/logging stubbed (anchor_point_test_stub.cpp); tinygettext
# include path is needed because gettext.hpp pulls in its header.
make_unit_test(AnchorPointTest SOURCE anchor_point_test.cpp anchor_point_test_stub.cpp
  LIBRARIES tinygettext
  EXTERNAL math/anchor_point.cpp
  INCLUDES ${SUPERTUX_SOURCE_DIR}/external/tinygettext/include ${CMAKE_BINARY_DIR})

# Coverage for math/bezier.hpp cubic-Bezier math (get_point/_raw, get_length,
# get_point_at_length/_by_length). The draw_curve() rendering path is not
# exercised. glm for Vector; logging stubbed (bezier_test_stub.cpp).
make_unit_test(BezierTest SOURCE bezier_test.cpp bezier_test_stub.cpp
  LIBRARIES SDL3::SDL3 glm
  DEFINITIONS GLM_ENABLE_EXPERIMENTAL
  EXTERNAL math/bezier.cpp video/color.cpp
  INCLUDES ${CMAKE_BINARY_DIR})

# TileReplacementTest: regression coverage for the editor tile-replacement
# degenerate/division-by-zero guard (issue #3810).
make_unit_test(TileReplacementTest SOURCE tile_replacement_test.cpp
  LIBRARIES GTest::gtest GTest::gtest_main SDL3::SDL3 glm
  EXTERNAL editor/tile_replacement.hpp)

# TileTest: coverage for supertux/tile.cpp attribute + solidity logic
# (is_solid / is_unisolid / is_slope flags, get_attributes / get_data,
# is_deprecated, object metadata, is_solid(tile_bbox,position,movement),
# is_collisionful). Pure logic; only tile.cpp is linked (no rendering), so the
# engine's Canvas/Editor/globals are stubbed (tile_test_stub.cpp).
make_unit_test(TileTest SOURCE tile_test.cpp tile_test_stub.cpp
  LIBRARIES GTest::gtest GTest::gtest_main SDL3::SDL3 glm
  DEFINITIONS GLM_ENABLE_EXPERIMENTAL
  EXTERNAL supertux/tile.cpp video/color.cpp math/rectf.cpp
  INCLUDES ${SUPERTUX_SOURCE_DIR}/external/tinygettext/include
           ${SUPERTUX_SOURCE_DIR}/external/sexp-cpp/include
           ${SUPERTUX_SOURCE_DIR}/external/simplesquirrel/include
           ${SUPERTUX_SOURCE_DIR}/external/simplesquirrel/libs/squirrel/include
           ${CMAKE_BINARY_DIR})
# PhysicMovementTest: covers the out-of-line Physic::get_movement()
# semi-implicit Euler integration (velocity + acceleration). With gravity
# disabled, get_movement() never touches Sector, so only physic.cpp is needed
# (no engine linkage). This complements PhysicTest, which covers the header-
# inline state logic.
make_unit_test(PhysicMovementTest SOURCE physic_movement_test.cpp physic_test_stub.cpp
  LIBRARIES GTest::gtest GTest::gtest_main
  DEFINITIONS GLM_ENABLE_EXPERIMENTAL
  EXTERNAL supertux/physic.cpp video/color.cpp
  INCLUDES ${SUPERTUX_SOURCE_DIR}/external/tinygettext/include
           ${SUPERTUX_SOURCE_DIR}/external/sexp-cpp/include
           ${SUPERTUX_SOURCE_DIR}/external/simplesquirrel/include
           ${SUPERTUX_SOURCE_DIR}/external/simplesquirrel/libs/squirrel/include
           ${CMAKE_BINARY_DIR})

# Same UBSan vptr caveat as PhysicTest: physic.cpp holds get_movement(), whose
# inline Sector::get() call references `typeinfo for Sector`; under UBSan the
# reference stays live. Sector is never instantiated here, so disable vptr
# sanitization on physic.cpp only.
target_compile_options(PhysicMovementTest PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-fno-sanitize=vptr>)

message("ALL TESTS: ${all_test_targets}")

add_custom_target(tests)
add_dependencies(tests ${all_test_targets})
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(check ${all_test_targets})
