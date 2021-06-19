cmake_minimum_required(VERSION 3.19.0)
project(tcob VERSION 0.0.1)

option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_TOOLS "Build tools" ON)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)

if(MSVC)
  add_definitions(/MP)
  add_definitions(/utf-8)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(/arch:AVX2)
  #set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=-*, modernize-*, performance-*, cppcore*, -cppcoreguidelines-avoid-magic-numbers,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-pro-type-reinterpret-cast;--extra-arg=/EHsc")
endif()

add_subdirectory(tcob)

if (BUILD_TESTS)
  add_subdirectory(tcob_tests)
endif()

if (BUILD_EXAMPLES)
  add_subdirectory(extras/examples)
endif()

if (BUILD_TOOLS)
  add_subdirectory(extras/tools)
endif()

MESSAGE(STATUS "Build examples: " ${BUILD_EXAMPLES})
MESSAGE(STATUS "Build tests: " ${BUILD_TESTS})
MESSAGE(STATUS "Build tools: " ${BUILD_TOOLS})
