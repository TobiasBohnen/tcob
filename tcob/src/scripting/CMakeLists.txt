# scripting
list(APPEND SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Scripting.cpp
)
list(APPEND HDR
    ${TCOB_INC_DIR}/tcob/scripting/Scripting.hpp
)

# Lua
if(TCOB_ENABLE_ADDON_SCRIPTING_LUA)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/Lua.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Script.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Types.cpp
    )
    list(APPEND HDR
        ${TCOB_INC_DIR}/tcob/scripting/Lua.hpp
        ${TCOB_INC_DIR}/tcob/scripting/Lua.inl
        ${TCOB_INC_DIR}/tcob/scripting/Closure.hpp
        ${TCOB_INC_DIR}/tcob/scripting/Closure.inl
        ${TCOB_INC_DIR}/tcob/scripting/Conversions.hpp
        ${TCOB_INC_DIR}/tcob/scripting/Script.hpp
        ${TCOB_INC_DIR}/tcob/scripting/Script.inl
        ${TCOB_INC_DIR}/tcob/scripting/Types.hpp
        ${TCOB_INC_DIR}/tcob/scripting/Types.inl
        ${TCOB_INC_DIR}/tcob/scripting/Wrapper.hpp
        ${TCOB_INC_DIR}/tcob/scripting/Wrapper.inl
    )
endif()

tcob_add_obj_library(tcob_scripting "${SRC}" "${HDR}")