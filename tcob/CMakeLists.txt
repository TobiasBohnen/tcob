add_subdirectory(extlibs)

# ##########################################################################
configure_file(
  ${TCOB_INC_DIR}/tcob/tcob_config.hpp.in
  ${TCOB_INC_DIR}/tcob/tcob_config.hpp
)

add_subdirectory(src/core)
list(APPEND LIBS tcob_core)

add_subdirectory(src/ai)
list(APPEND LIBS tcob_ai)

add_subdirectory(src/audio)
list(APPEND LIBS tcob_audio)

add_subdirectory(src/data)
list(APPEND LIBS tcob_data)

add_subdirectory(src/gfx)
list(APPEND LIBS tcob_gfx)

add_subdirectory(src/physics)
list(APPEND LIBS tcob_physics)

add_subdirectory(src/scripting)
list(APPEND LIBS tcob_scripting)

add_subdirectory(src/app)
list(APPEND LIBS tcob_app)

set(TCOB_LIBS ${LIBS} PARENT_SCOPE)

# ##########################################################################
# ##########################################################################
if(TCOB_BUILD_SHARED)
  add_library(tcob_shared SHARED)
  target_include_directories(tcob_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
  target_link_libraries(tcob_shared PUBLIC ${LIBS})
  set_target_properties(tcob_shared PROPERTIES DEBUG_POSTFIX "_d")
else()
  add_library(tcob_static STATIC)
  target_include_directories(tcob_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
  target_link_libraries(tcob_static PUBLIC ${LIBS})
  set_target_properties(tcob_static PROPERTIES DEBUG_POSTFIX "_d")
endif()
