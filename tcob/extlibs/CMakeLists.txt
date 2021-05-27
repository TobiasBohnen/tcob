#SDL2
find_package(SDL2 CONFIG REQUIRED)

#WebP
find_package(WebP CONFIG REQUIRED)

#Lua
file(GLOB LUA_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/lua/*.cpp)

#physfs
file(GLOB PHYSFS_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/physfs/*.c)


###########################################################################

add_library(tcob_extlibs INTERFACE)

target_sources(tcob_extlibs INTERFACE 
    AL/mojoal.c

    glad/gl.c

    spng/spng.c

    miniz/miniz.c

    dr_libs/dr_libs.c
    
    stb/stb_vorbis.c
    
    ${LUA_SRC_FILES}
    ${PHYSFS_SRC_FILES}
)

target_compile_definitions(tcob_extlibs 
    INTERFACE SPNG_USE_MINIZ 
    INTERFACE SPNG_STATIC
    INTERFACE AL_LIBTYPE_STATIC)

target_include_directories(tcob_extlibs 
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} 
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lua/
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/physfs/)

target_link_libraries(tcob_extlibs 
    INTERFACE SDL2::SDL2-static 
    INTERFACE WebP::webpdemux)