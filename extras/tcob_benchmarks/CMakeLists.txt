Include(FetchContent)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark
  GIT_TAG v1.8.0
)

if(MSVC)
  set(HAVE_POSIX_REGEX OFF)
endif()

set(BENCHMARK_USE_BUNDLED_GTEST OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(BENCHMARK_ENABLE_TESTING OFF)

FetchContent_GetProperties(benchmark)

if(NOT benchmark_POPULATED)
  FetchContent_Populate(benchmark)
  add_subdirectory(${benchmark_SOURCE_DIR} ${benchmark_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable(tcob_benchmarks)

set(BENCH_SRCFILES
  main.cpp
  ImageBenchmarks.cpp
  RandomBenchmarks.cpp
  TransformBenchmarks.cpp
)

if(TCOB_ENABLE_ADDON_SCRIPTING_LUA)
  list(APPEND BENCH_SRCFILES
    DataObjectBenchmarks.cpp
  )
endif()

if(TCOB_ENABLE_ADDON_SCRIPTING_LUA)
  list(APPEND BENCH_SRCFILES
    LuaScriptBenchmarks.cpp
  )
endif()

target_compile_definitions(tcob_benchmarks PRIVATE BENCHMARK_STATIC_DEFINE)

target_sources(tcob_benchmarks PRIVATE ${BENCH_SRCFILES})

if(NOT TCOB_BUILD_SHARED)
  target_link_libraries(tcob_benchmarks PRIVATE benchmark::benchmark PRIVATE tcob_static)
else()
  target_link_libraries(tcob_benchmarks PRIVATE benchmark::benchmark PRIVATE tcob_shared)
endif()

target_include_directories(tcob_benchmarks PRIVATE . PRIVATE ../../tcob/include)

set_target_properties(tcob_benchmarks PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED TRUE
)

file(COPY testfiles DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")