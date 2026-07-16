# Will eventually handle game tests here too
include(CTest)

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

make_unit_test(MD5Test SOURCE md5_test.cpp
  EXTERNAL addon/md5.cpp)

make_unit_test(AATriangleTest SOURCE aatriangle_test.cpp
  EXTERNAL math/aatriangle.cpp
  LIBRARIES SDL2 glm DEFINITIONS GLM_ENABLE_EXPERIMENTAL)

make_unit_test(CollisionTest SOURCE collision_test.cpp
  EXTERNAL math/rectf.cpp
  LIBRARIES SDL2 glm DEFINITIONS GLM_ENABLE_EXPERIMENTAL)

make_unit_test(DynamicScopedTest SOURCE dynamic_scoped_test.cpp
  GTEST)

# The following tests pull in large engine subsystems (tinygettext/gettext/SDL GUI
# via util/file_system.cpp and editor/object_option.cpp). They are kept in the tree
# but not yet wired into CTest until their transitive dependencies (tinygettext,
# gettext, SDL renderer) are linked. Re-enable once those libs are available here.
# make_unit_test(FileSystemTest SOURCE file_system_test.cpp
#   GTEST LIBRARIES SDL2 EXTERNAL util/file_system.cpp)
#
# make_unit_test(IFileStreamTest SOURCE ifile_stream_test.cpp
#   GTEST SDL2 LIBRARIES physfs EXTERNAL physfs/ifile_stream.cpp physfs/ifile_streambuf.cpp physfs/util.cpp)
# NOTE: physfs/util.cpp pulls in physfs/physfs_file_system.hpp which requires
# tinygettext; enabling this test needs tinygettext (and the rest of the engine
# libraries) linked first. Left disabled until the engine is available to tests.
#
# make_unit_test(ObjectOptionTest SOURCE object_option_test.cpp
#   GTEST EXTERNAL editor/object_option.cpp supertux/game_object.cpp video/color.cpp)
#
# make_unit_test(ReaderTest SOURCE reader_test.cpp
#   GTEST EXTERNAL util/reader_document.cpp util/reader_mapping.cpp util/reader_collection.cpp util/reader_object.cpp physfs/ifile_stream.cpp util/file_system.cpp util/log.cpp)

make_unit_test(RandomTest SOURCE random_test.cpp
  GTEST EXTERNAL math/random.cpp)

make_unit_test(RectTest SOURCE rect_test.cpp
  GTEST SDL2 EXTERNAL math/rect.cpp)

make_unit_test(RectfTest SOURCE rectf_test.cpp
  GTEST SDL2 EXTERNAL math/rectf.cpp)

make_unit_test(SizeTest SOURCE size_test.cpp
  GTEST SDL2 EXTERNAL math/size.cpp)

make_unit_test(SizefTest SOURCE sizef_test.cpp
  GTEST SDL2 EXTERNAL math/sizef.cpp)

make_unit_test(StringUtilTest SOURCE string_util_test.cpp
  GTEST EXTERNAL util/string_util.cpp)

# UidTest pulls in util/log.cpp, which transitively requires the full engine
# (simplesquirrel via supertux/console.hpp). It is kept in the tree but not
# wired into CTest until the test can link against the engine libraries.
# make_unit_test(UidTest SOURCE uid_test.cpp
#   GTEST SDL2 EXTERNAL util/uid_generator.cpp util/log.cpp)

make_unit_test(VersionTest SOURCE version_test.cpp
  GTEST INCLUDES ${CMAKE_BINARY_DIR})

message("ALL TESTS: ${all_test_targets}")

add_custom_target(tests DEPENDS ${all_test_targets})
add_custom_command(TARGET tests POST_BUILD COMMAND ctest -C $<CONFIGURATION> --output-on-failure)
