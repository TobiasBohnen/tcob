add_executable(png2array)

target_sources(png2array PRIVATE main.cpp)

set_target_properties(png2array PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
)

if(TCOB_BUILD_SHARED)
    target_link_libraries(png2array PRIVATE tcob_shared)
else()
    target_link_libraries(png2array PRIVATE tcob_static)
endif()
