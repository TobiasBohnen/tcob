cmake_minimum_required(VERSION 3.19.0)
project(tcob VERSION 0.0.1)

option(TCOB_BUILD_EXAMPLES "Build examples" ON)
option(TCOB_BUILD_TESTS "Build tests" ON)
option(TCOB_BUILD_TOOLS "Build tools" ON)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)



if(MSVC)
  add_compile_options(/utf-8)
  add_compile_options(/arch:AVX2)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
  #set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=-*, modernize-*, performance-*, cppcore*, -cppcoreguidelines-avoid-magic-numbers,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-pro-type-reinterpret-cast;--extra-arg=/EHsc")
endif()

add_subdirectory(tcob)

if (TCOB_BUILD_TESTS)
  add_subdirectory(tcob_tests)
endif()

if (TCOB_BUILD_EXAMPLES)
  add_subdirectory(extras/examples)
endif()

if (TCOB_BUILD_TOOLS)
  add_subdirectory(extras/tools)
endif()

MESSAGE(STATUS "Build examples: " ${TCOB_BUILD_EXAMPLES})
MESSAGE(STATUS "Build tests: " ${TCOB_BUILD_TESTS})
MESSAGE(STATUS "Build tools: " ${TCOB_BUILD_TOOLS})
