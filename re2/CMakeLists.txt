# Copyright 2015 The RE2 Authors.  All Rights Reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

# Old enough to support Ubuntu Bionic.
cmake_minimum_required(VERSION 3.10.2)

project(src/re2 CXX)
include(CMakePackageConfigHelpers)
include(CTest)
include(GNUInstallDirs)

option(BUILD_SHARED_LIBS "build shared libraries" OFF)
option(USEPCRE "use PCRE in tests and benchmarks" OFF)

# CMake seems to have no way to enable/disable testing per subproject,
# so we provide an option similar to BUILD_TESTING, but just for RE2.
option(RE2_BUILD_TESTING "enable testing for RE2" ON)

# ABI version
# http://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html
set(SONAME 10)

set(EXTRA_TARGET_LINK_LIBRARIES)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  if(MSVC_VERSION LESS 1900)
    message(FATAL_ERROR "you need Visual Studio 2015 or later")
  endif()
  if(BUILD_SHARED_LIBS)
    # See http://www.kitware.com/blog/home/post/939 for details.
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  endif()
  # CMake defaults to /W3, but some users like /W4 (or /Wall) and /WX,
  # so we disable various warnings that aren't particularly helpful.
  add_compile_options(/wd4100 /wd4201 /wd4456 /wd4457 /wd4702 /wd4815)
  # Without a byte order mark (BOM), Visual Studio assumes that the source
  # file is encoded using the current user code page, so we specify UTF-8.
  add_compile_options(/utf-8)
endif()

if(WIN32)
  add_definitions(-DUNICODE -D_UNICODE -DSTRICT -DNOMINMAX)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
elseif(UNIX)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
endif()

if(USEPCRE)
  add_definitions(-DUSEPCRE)
  list(APPEND EXTRA_TARGET_LINK_LIBRARIES pcre)
endif()

set(RE2_SOURCES
        src/re2/bitstate.cc
        src/re2/compile.cc
        src/re2/dfa.cc
        src/re2/filtered_re2.cc
        src/re2/mimics_pcre.cc
        src/re2/nfa.cc
        src/re2/onepass.cc
        src/re2/parse.cc
        src/re2/perl_groups.cc
        src/re2/prefilter.cc
        src/re2/prefilter_tree.cc
        src/re2/prog.cc
        src/re2/re2.cc
        src/re2/regexp.cc
        src/re2/set.cc
        src/re2/simplify.cc
        src/re2/stringpiece.cc
        src/re2/tostring.cc
        src/re2/unicode_casefold.cc
        src/re2/unicode_groups.cc
        src/util/rune.cc
        src/util/strutil.cc
    )

add_library(src/re2 ${RE2_SOURCES})
target_compile_features(src/re2 PUBLIC cxx_std_11)
target_include_directories(src/re2 PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
set_target_properties(src/re2 PROPERTIES SOVERSION ${SONAME} VERSION ${SONAME}.0.0)
add_library(re2::re2 ALIAS src/re2)

if(UNIX)
  target_link_libraries(src/re2 PUBLIC Threads::Threads)
endif()

if(RE2_BUILD_TESTING)
  set(TESTING_SOURCES
          src/re2/testing/backtrack.cc
          src/re2/testing/dump.cc
          src/re2/testing/exhaustive_tester.cc
          src/re2/testing/null_walker.cc
          src/re2/testing/regexp_generator.cc
          src/re2/testing/string_generator.cc
          src/re2/testing/tester.cc
          src/util/pcre.cc
      )

  add_library(testing STATIC ${TESTING_SOURCES})
  target_compile_features(testing PUBLIC cxx_std_11)
  target_link_libraries(testing PUBLIC src/re2)

  set(TEST_TARGETS
      charclass_test
      compile_test
      filtered_re2_test
      mimics_pcre_test
      parse_test
      possible_match_test
      re2_test
      re2_arg_test
      regexp_test
      required_prefix_test
      search_test
      set_test
      simplify_test
      string_generator_test

      dfa_test
      exhaustive1_test
      exhaustive2_test
      exhaustive3_test
      exhaustive_test
      random_test
      )

  set(BENCHMARK_TARGETS
      regexp_benchmark
      )

  foreach(target ${TEST_TARGETS})
    add_executable(${target} src/re2/testing/${target}.cc src/util/test.cc)
    target_compile_features(${target} PUBLIC cxx_std_11)
    target_link_libraries(${target} testing ${EXTRA_TARGET_LINK_LIBRARIES})
    add_test(NAME ${target} COMMAND ${target})
  endforeach()

  foreach(target ${BENCHMARK_TARGETS})
    add_executable(${target} src/re2/testing/${target}.cc src/util/benchmark.cc)
    target_compile_features(${target} PUBLIC cxx_std_11)
    target_link_libraries(${target} testing ${EXTRA_TARGET_LINK_LIBRARIES})
  endforeach()
endif()

set(RE2_HEADERS
        src/re2/filtered_re2.h
        src/re2/re2.h
        src/re2/set.h
        src/re2/stringpiece.h
    )

install(FILES ${RE2_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/re2)
install(TARGETS src/re2 EXPORT re2Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT re2Targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/re2 NAMESPACE re2::)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/re2Config.cmake.in
                              ${CMAKE_CURRENT_BINARY_DIR}/re2Config.cmake
                              INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/re2
                              )
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/re2ConfigVersion.cmake
                                 VERSION ${SONAME}.0.0
                                 COMPATIBILITY SameMajorVersion
                                 )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/re2Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/re2ConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/re2)
