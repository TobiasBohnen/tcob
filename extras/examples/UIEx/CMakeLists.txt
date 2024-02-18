add_executable(UIEx)

target_sources(UIEx PRIVATE
    main.cpp
    UIEx.cpp
    ExStyle_Color.cpp
    ExStyle_Skin.cpp
    ExForms.cpp
)

set_target_properties(UIEx PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
)

if(NOT TCOB_BUILD_SHARED)
    target_link_libraries(UIEx PRIVATE tcob_static)
else()
    target_link_libraries(UIEx PRIVATE tcob_shared)
endif()

target_include_directories(UIEx PRIVATE ../../../tcob/include)

if(TCOB_BUILD_SHARED AND NOT TCOB_IS_CI)
    add_custom_command(TARGET UIEx POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:UIEx> ${CMAKE_BINARY_DIR}/../../x64/
        COMMAND_EXPAND_LISTS
    )
endif()

configure_file(uiex.zip ${CMAKE_BINARY_DIR}/../../x64/uiex.zip COPYONLY)
