// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/script/LuaState.hpp>

#include <lua.hpp>

#include <tcob/core/io/Logger.hpp>

namespace tcob {

LuaStackGuard::LuaStackGuard(lua_State* l)
    : _luaState { l }
    , _oldTop { lua_gettop(l) }
{
}

LuaStackGuard::~LuaStackGuard()
{
    const i32 n { lua_gettop(_luaState) - _oldTop };
    if (n > 0)
        lua_pop(_luaState, n);
}

////////////////////////////////////////////////////////////

LuaState::LuaState(lua_State* l)
    : _luaState { l }
{
}

auto LuaState::is_bool(i32 idx) const -> bool
{
    return lua_isboolean(_luaState, idx);
}

auto LuaState::is_function(i32 idx) const -> bool
{
    return lua_isfunction(_luaState, idx);
}

auto LuaState::is_integer(i32 idx) const -> bool
{
    return lua_isinteger(_luaState, idx);
}

auto LuaState::is_number(i32 idx) const -> bool
{
    return lua_isnumber(_luaState, idx);
}

auto LuaState::is_string(i32 idx) const -> bool
{
    return lua_isstring(_luaState, idx);
}

auto LuaState::is_table(i32 idx) const -> bool
{
    return lua_istable(_luaState, idx);
}

auto LuaState::is_thread(i32 idx) const -> bool
{
    return lua_isthread(_luaState, idx);
}

auto LuaState::is_nil(i32 idx) const -> bool
{
    return lua_isnil(_luaState, idx);
}

auto LuaState::is_userdata(i32 idx) const -> bool
{
    return lua_isuserdata(_luaState, idx);
}

auto LuaState::to_bool(i32 idx) const -> bool
{
    return lua_toboolean(_luaState, idx);
}

auto LuaState::to_integer(i32 idx) const -> i64
{
    return lua_tointeger(_luaState, idx);
}

auto LuaState::to_number(i32 idx) const -> f64
{
    return lua_tonumber(_luaState, idx);
}

auto LuaState::to_string(i32 idx) const -> const char*
{
    return lua_tostring(_luaState, idx);
}

auto LuaState::to_userdata(i32 idx) const -> void*
{
    return lua_touserdata(_luaState, idx);
}

auto LuaState::to_thread(i32 idx) const -> LuaState
{
    return LuaState { lua_tothread(_luaState, idx) };
}

auto LuaState::get_type(i32 idx) const -> LuaType
{
    switch (lua_type(_luaState, idx)) {
    case LUA_TNONE:
        return LuaType::None;
    case LUA_TNIL:
        return LuaType::Nil;
    case LUA_TBOOLEAN:
        return LuaType::Boolean;
    case LUA_TLIGHTUSERDATA:
        return LuaType::LightUserdata;
    case LUA_TNUMBER:
        return LuaType::Number;
    case LUA_TSTRING:
        return LuaType::String;
    case LUA_TTABLE:
        return LuaType::Table;
    case LUA_TFUNCTION:
        return LuaType::Function;
    case LUA_TUSERDATA:
        return LuaType::Userdata;
    case LUA_TTHREAD:
        return LuaType::Thread;
    default:
        return LuaType::None;
    }
}

auto LuaState::get_top() const -> i32
{
    return lua_gettop(_luaState);
}

void LuaState::check_stack(i32 size) const
{
    lua_checkstack(_luaState, size);
}

auto LuaState::resume(i32 argCount, i32* resultCount) const -> LuaThreadState
{
    const i32 err { lua_resume(_luaState, nullptr, argCount, resultCount) };
    switch (err) {
    case LUA_OK:
        return LuaThreadState::Ok;
    case LUA_YIELD:
        return LuaThreadState::Yielded;
    case LUA_ERRRUN:
        return LuaThreadState::RuntimeError;
    case LUA_ERRSYNTAX:
        return LuaThreadState::SyntaxError;
    case LUA_ERRMEM:
        return LuaThreadState::MemError;
    case LUA_ERRERR:
        return LuaThreadState::ErrorError;
    }

    return LuaThreadState::ErrorError;
}

auto LuaState::next(i32 idx) const -> bool
{
    return lua_next(_luaState, idx) != 0;
}

void LuaState::push_bool(bool val) const
{
    lua_pushboolean(_luaState, val);
}

void LuaState::push_cfunction(i32 (*lua_CFunction)(lua_State*)) const
{
    lua_pushcfunction(_luaState, lua_CFunction);
}

void LuaState::push_cclosure(i32 (*lua_CFunction)(lua_State*), i32 n) const
{
    lua_pushcclosure(_luaState, lua_CFunction, n);
}

void LuaState::push_integer(i64 val) const
{
    lua_pushinteger(_luaState, val);
}

void LuaState::push_lightuserdata(void* p) const
{
    lua_pushlightuserdata(_luaState, p);
}

void LuaState::push_nil() const
{
    lua_pushnil(_luaState);
}

void LuaState::push_number(f64 val) const
{
    lua_pushnumber(_luaState, val);
}

void LuaState::push_string(const char* val) const
{
    lua_pushstring(_luaState, val);
}

void LuaState::push_value(i32 idx) const
{
    lua_pushvalue(_luaState, idx);
}

void LuaState::pop(i32 count) const
{
    lua_pop(_luaState, count);
}

void LuaState::remove(i32 count) const
{
    lua_remove(_luaState, count);
}

void LuaState::get_table(i32 idx) const
{
    lua_gettable(_luaState, idx);
}

void LuaState::get_metatable(const char* tableName) const
{
    luaL_getmetatable(_luaState, tableName);
}

void LuaState::set_table(i32 idx) const
{
    lua_settable(_luaState, idx);
}

void LuaState::set_metatable(i32 idx) const
{
    lua_setmetatable(_luaState, idx);
}

void LuaState::new_table() const
{
    lua_newtable(_luaState);
}
void LuaState::create_table(i32 narr, i32 nrec) const
{
    lua_createtable(_luaState, narr, nrec);
}
auto LuaState::new_metatable(const char* tableName) const -> i32
{
    return luaL_newmetatable(_luaState, tableName);
}
auto LuaState::new_userdata(i32 size) const -> void*
{
    return lua_newuserdatauv(_luaState, size, 0);
}
auto LuaState::new_userdata(i32 size, i32 nuvalue) const -> void*
{
    return lua_newuserdatauv(_luaState, size, nuvalue);
}

void LuaState::set_registry_field(const char* name) const
{
    lua_setfield(_luaState, LUA_REGISTRYINDEX, name);
}

void LuaState::insert(i32 idx) const
{
    lua_insert(_luaState, idx);
}

auto LuaState::raw_len(i32 idx) const -> u64
{
    return lua_rawlen(_luaState, idx);
}

void LuaState::raw_get(i32 idx, i64 n) const
{
    lua_rawgeti(_luaState, idx, n);
}

void LuaState::raw_get(i32 idx) const
{
    lua_rawget(_luaState, idx);
}

void LuaState::raw_set(i32 idx, i64 n) const
{
    lua_rawseti(_luaState, idx, n);
}

void LuaState::raw_set(i32 idx) const
{
    lua_rawset(_luaState, idx);
}

auto LuaState::get_uservalue(i32 index, i32 n) const -> i32
{
    return lua_getiuservalue(_luaState, index, n);
}
auto LuaState::set_uservalue(i32 index, i32 n) const -> i32
{
    return lua_setiuservalue(_luaState, index, n);
}

auto LuaState::UpvalueIndex(i32 n) -> i32
{
    return lua_upvalueindex(n);
}

auto LuaState::lua() const -> lua_State*
{
    return _luaState;
}

auto LuaState::create_stack_guard() const -> LuaStackGuard
{
    return LuaStackGuard { _luaState };
}

auto LuaState::ref(i32 idx) const -> i32
{
    return luaL_ref(_luaState, idx);
}

void LuaState::unref(i32 t, i32 ref) const
{
    luaL_unref(_luaState, t, ref);
}

auto LuaState::status() const -> i32
{
    return lua_status(_luaState);
}

auto LuaState::reset_thread() const -> i32
{
    return lua_resetthread(_luaState);
}

auto LuaState::do_call(i32 nargs, i32 nret) const -> LuaResultState
{
    const i32 hpos { get_top() - nargs };

    push_cfunction(
        [](lua_State* l) -> i32 {
                const i32 n { lua_gettop(l)};
                Log("Lua says: " + std::string(lua_tostring(l, n)), LogLevel::Error);
                return 0; });
    insert(hpos);
    const i32 err { lua_pcall(lua(), nargs, nret, hpos) };
    remove(hpos);

    switch (err) {
    case LUA_ERRRUN:
        return LuaResultState::RuntimeError;
    case LUA_ERRMEM:
        return LuaResultState::MemAllocError;
    case LUA_OK:
        return LuaResultState::Ok;
    default:
        return LuaResultState::RuntimeError;
    }
}

}
