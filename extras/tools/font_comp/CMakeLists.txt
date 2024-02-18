add_executable(font_comp)

target_sources(font_comp PRIVATE
    main.cpp
)

set_target_properties(font_comp PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
)

if(TCOB_BUILD_SHARED)
    target_link_libraries(font_comp PRIVATE tcob_shared)
else()
    target_link_libraries(font_comp PRIVATE tcob_static)
endif()
