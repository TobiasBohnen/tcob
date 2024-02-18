add_executable(cia_conv)

target_sources(cia_conv PRIVATE
    main.cpp
    convert.cpp
)

set_target_properties(cia_conv PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
)

if(TCOB_BUILD_SHARED)
    target_link_libraries(cia_conv PRIVATE tcob_shared)
else()
    target_link_libraries(cia_conv PRIVATE tcob_static)
endif()
