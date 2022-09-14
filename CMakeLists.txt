cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# SETTINGS
set(ASMJIT_DIRECTORY "${CMAKE_SOURCE_DIR}/asmjit")
set(RE2_DIRECTORY "${CMAKE_SOURCE_DIR}/re2")

set(PLATFORM "WINDOWS")
set(PROFILE "DEBUG")

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${ASMJIT_DIRECTORY}/src
    ${RE2_DIRECTORY}/src
    ${RE2_DIRECTORY}/src/re2/fuzzing/compiler-rt/include
)

project(rebar)

add_compile_definitions(COMPILER_PROFILE=${COMPILER_PROFILE})

if ("${PLATFORM}" STREQUAL "WINDOWS")
    add_compile_definitions(REBAR_PLATFORM_WINDOWS)
elseif("${PLATFORM}" STREQUAL "APPLE")
    add_compile_definitions(REBAR_PLATFORM_APPLE)
elseif("${PLATFORM}" STREQUAL "LINUX")
    add_compile_definitions(REBAR_PLATFORM_LINUX)
endif()

if ("${PROFILE}" STREQUAL "DEBUG")
    add_compile_definitions(REBAR_DEBUG)
endif()

file(GLOB_RECURSE ASMJIT_SOURCE_FILES "${ASMJIT_DIRECTORY}/src/*.cpp" "${ASMJIT_DIRECTORY}/src/*.c")
file(GLOB_RECURSE RE2_SOURCE_FILES "${RE2_DIRECTORY}/src/*.cc")

add_executable(test64 ${CMAKE_SOURCE_DIR}/test/main.cpp ${ASMJIT_SOURCE_FILES} ${RE2_SOURCE_FILES})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GCC" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_definitions(-pthread -Wall -Wextra -Wconversion -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wcast-qual -Wunused -Woverloaded-virtual -Wno-noexcept-type -Wpedantic -fsanitize=address -fsanitize=undefined -msse2 -m64 -g)

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_definitions(-Weverything -Wno-float-equal -Wno-c++98-compat-pedantic -Wno-c++98-compat -Wno-documentation -Wno-switch-enum -Wno-weak-vtables -Wno-missing-prototypes -Wno-padded -Wno-missing-noreturn -Wno-exit-time-destructors -Wno-documentation-unknown-command -Wno-unused-template -Wno-undef)
    else()
        add_definitions(-Wnoexcept)
    endif()

    set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=address -fsanitize=undefined")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_definitions(/W4)
endif()