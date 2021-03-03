cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(rebar)

add_executable(test64 ${CMAKE_SOURCE_DIR}/test/main.cpp)

target_include_directories(test64 PUBLIC "${CMAKE_SOURCE_DIR}/src")

add_definitions(-pthread -Wall -Wextra -Wconversion -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wcast-qual -Wunused -Woverloaded-virtual -Wno-noexcept-type -Wpedantic -fsanitize=address -fsanitize=undefined -g)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_definitions(-Weverything -Wno-c++98-compat-pedantic -Wno-c++98-compat -Wno-documentation -Wno-switch-enum -Wno-weak-vtables -Wno-missing-prototypes -Wno-padded -Wno-missing-noreturn -Wno-exit-time-destructors -Wno-documentation-unknown-command -Wno-unused-template -Wno-undef)
else()
    add_definitions(-Wnoexcept)
endif()

set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=address -fsanitize=undefined")