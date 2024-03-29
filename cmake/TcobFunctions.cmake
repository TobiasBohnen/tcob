function(tcob_add_obj_library module sources headers)
    add_library(${module} OBJECT ${sources})

    target_sources(${module}
        PUBLIC FILE_SET HEADERS
        BASE_DIRS ${TCOB_INC_DIR}
        FILES ${headers}
    )

    target_include_directories(${module}
        PUBLIC
        ${TCOB_INC_DIR}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/
    )

    target_compile_options(${module} PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<BOOL:TCOB_USE_DEFAULT_MSVC_OPTIONS>>: /W4>
        $<$<CXX_COMPILER_ID:Clang>: -Wall -Wextra
        -Wno-float-equal -Wno-double-promotion -Wno-implicit-int-float-conversion -Wno-sign-conversion -Wno-switch-enum -Wno-switch-default
        -Wno-unsafe-buffer-usage -Wno-ctad-maybe-unsupported -Wno-c++20-compat -Wno-c++98-compat -Wno-c++98-compat-pedantic
        -Wno-exit-time-destructors>
        $<$<CXX_COMPILER_ID:GNU>: -Wall -Wextra -pedantic
        -Wno-missing-field-initializers>
    )

    set_target_properties(${module} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
    )

    if(${module} STREQUAL "tcob_core")
        target_link_libraries(${module} PUBLIC tcob_extlibs)
        target_precompile_headers(${module} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:../_pch.hpp>)
    else()
        target_link_libraries(${module} PUBLIC tcob_core)
        target_precompile_headers(${module} REUSE_FROM tcob_core)
    endif()

    if(TCOB_BUILD_SHARED)
        target_compile_definitions(${module} PRIVATE TCOB_EXPORTS)
    endif()
endfunction()

function(tcob_mark_all_as_advanced name)
    get_cmake_property(_variableNames VARIABLES)
    list(SORT _variableNames)

    foreach(_variableName ${_variableNames})
        if(_variableName MATCHES ${name})
            mark_as_advanced(FORCE ${_variableName})
        endif()
    endforeach()
endfunction()

function(tcob_disable_warnings target)
    target_compile_options(${target} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/w>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-everything>
        $<$<CXX_COMPILER_ID:GNU>:-w>
    )
endfunction()