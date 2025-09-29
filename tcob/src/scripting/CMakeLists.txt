# scripting
list(APPEND SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Scripting.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Lua.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LuaScript.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LuaTypes.cpp
)
list(APPEND HDR
    ${TCOB_INC_DIR}/tcob/scripting/Scripting.hpp
    ${TCOB_INC_DIR}/tcob/scripting/Lua.hpp
    ${TCOB_INC_DIR}/tcob/scripting/Lua.inl
    ${TCOB_INC_DIR}/tcob/scripting/LuaConversions.hpp
    ${TCOB_INC_DIR}/tcob/scripting/LuaClosure.hpp
    ${TCOB_INC_DIR}/tcob/scripting/LuaClosure.inl
    ${TCOB_INC_DIR}/tcob/scripting/LuaScript.hpp
    ${TCOB_INC_DIR}/tcob/scripting/LuaScript.inl
    ${TCOB_INC_DIR}/tcob/scripting/LuaTypes.hpp
    ${TCOB_INC_DIR}/tcob/scripting/LuaTypes.inl
    ${TCOB_INC_DIR}/tcob/scripting/LuaWrapper.hpp
    ${TCOB_INC_DIR}/tcob/scripting/LuaWrapper.inl
)

tcob_add_obj_library(tcob_scripting "${SRC}" "${HDR}")