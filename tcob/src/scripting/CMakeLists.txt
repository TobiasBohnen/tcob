# scripting
list(APPEND SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Scripting.cpp
)
list(APPEND HDR
    ${TCOB_INC_DIR}/tcob/scripting/Script.hpp
    ${TCOB_INC_DIR}/tcob/scripting/Script.inl
    ${TCOB_INC_DIR}/tcob/scripting/Scripting.hpp
    ${TCOB_INC_DIR}/tcob/scripting/Wrapper.hpp
    ${TCOB_INC_DIR}/tcob/scripting/Wrapper.inl
)

# Lua
if(TCOB_ENABLE_ADDON_SCRIPTING_LUA)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/lua/Lua.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lua/LuaScript.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lua/LuaTypes.cpp
    )
    list(APPEND HDR
        ${TCOB_INC_DIR}/tcob/scripting/lua/Lua.hpp
        ${TCOB_INC_DIR}/tcob/scripting/lua/Lua.inl
        ${TCOB_INC_DIR}/tcob/scripting/lua/LuaClosure.hpp
        ${TCOB_INC_DIR}/tcob/scripting/lua/LuaClosure.inl
        ${TCOB_INC_DIR}/tcob/scripting/lua/LuaConversions.inl
        ${TCOB_INC_DIR}/tcob/scripting/lua/LuaScript.hpp
        ${TCOB_INC_DIR}/tcob/scripting/lua/LuaScript.inl
        ${TCOB_INC_DIR}/tcob/scripting/lua/LuaTypes.hpp
        ${TCOB_INC_DIR}/tcob/scripting/lua/LuaTypes.inl
        ${TCOB_INC_DIR}/tcob/scripting/lua/LuaWrapper.hpp
        ${TCOB_INC_DIR}/tcob/scripting/lua/LuaWrapper.inl
    )
endif()

# Squirrel
if(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/squirrel/Squirrel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/squirrel/SquirrelScript.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/squirrel/SquirrelTypes.cpp
    )
    list(APPEND HDR
        ${TCOB_INC_DIR}/tcob/scripting/squirrel/Squirrel.hpp
        ${TCOB_INC_DIR}/tcob/scripting/squirrel/Squirrel.inl
        ${TCOB_INC_DIR}/tcob/scripting/squirrel/SquirrelClosure.hpp
        ${TCOB_INC_DIR}/tcob/scripting/squirrel/SquirrelClosure.inl
        ${TCOB_INC_DIR}/tcob/scripting/squirrel/SquirrelConversions.inl
        ${TCOB_INC_DIR}/tcob/scripting/squirrel/SquirrelScript.hpp
        ${TCOB_INC_DIR}/tcob/scripting/squirrel/SquirrelScript.inl
        ${TCOB_INC_DIR}/tcob/scripting/squirrel/SquirrelTypes.hpp
        ${TCOB_INC_DIR}/tcob/scripting/squirrel/SquirrelTypes.inl
        ${TCOB_INC_DIR}/tcob/scripting/squirrel/SquirrelWrapper.hpp
        ${TCOB_INC_DIR}/tcob/scripting/squirrel/SquirrelWrapper.inl
    )
endif()

tcob_add_obj_library(tcob_scripting "${SRC}" "${HDR}")