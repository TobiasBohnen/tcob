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
        $<$<CXX_COMPILER_ID:MSVC>: /W4>
        $<$<CXX_COMPILER_ID:Clang>: -Wall -Wextra

        -Wno-sign-conversion # TODO (but probably won't)
        -Wno-double-promotion # WONTFIX
        -Wno-float-equal # WONTFIX
        -Wno-switch-default -Wno-switch-enum # WONTFIX
        -Wno-exit-time-destructors # WONTFIX
        -Wno-unsafe-buffer-usage # WONTFIX
        -Wno-ctad-maybe-unsupported # WONTFIX
        -Wno-c++98-compat-pedantic -Wno-c++20-compat-pedantic # WONTFIX
        >
        $<$<CXX_COMPILER_ID:GNU>: -Wall -Wextra -pedantic -Wno-missing-field-initializers>
    )

    if(EMSCRIPTEN)
        target_compile_options(${module} PRIVATE -fexperimental-library -pthread)
        target_link_options(${module} PRIVATE -sFULL_ES3 -pthread)
    endif()

    set_target_properties(${module} PROPERTIES
        CXX_STANDARD 23
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
    if(MSVC)
        target_compile_options(${target} PRIVATE /W0 /w)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${target} PRIVATE -Wno-everything)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE -w)
    endif()
endfunction()