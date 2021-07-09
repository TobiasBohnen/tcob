// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/script/LuaState.hpp>

#include <lua.hpp>

#include <tcob/core/io/Logger.hpp>

namespace tcob::lua {

StackGuard::StackGuard(lua_State* l)
    : _luaState { l }
    , _oldTop { lua_gettop(l) }
{
}

StackGuard::~StackGuard()
{
    const i32 n { lua_gettop(_luaState) - _oldTop };
    if (n > 0)
        lua_pop(_luaState, n);
}

////////////////////////////////////////////////////////////

auto convert_type(i32 i) -> Type
{
    switch (i) {
    case LUA_TNONE:
        return Type::None;
    case LUA_TNIL:
        return Type::Nil;
    case LUA_TBOOLEAN:
        return Type::Boolean;
    case LUA_TLIGHTUSERDATA:
        return Type::LightUserdata;
    case LUA_TNUMBER:
        return Type::Number;
    case LUA_TSTRING:
        return Type::String;
    case LUA_TTABLE:
        return Type::Table;
    case LUA_TFUNCTION:
        return Type::Function;
    case LUA_TUSERDATA:
        return Type::Userdata;
    case LUA_TTHREAD:
        return Type::Thread;
    default:
        return Type::None;
    }
}

State::State(lua_State* l)
    : _luaState { l }
{
}

auto State::is_bool(i32 idx) const -> bool
{
    return lua_isboolean(_luaState, idx);
}

auto State::is_function(i32 idx) const -> bool
{
    return lua_isfunction(_luaState, idx);
}

auto State::is_integer(i32 idx) const -> bool
{
    return lua_isinteger(_luaState, idx);
}

auto State::is_number(i32 idx) const -> bool
{
    return lua_isnumber(_luaState, idx);
}

auto State::is_string(i32 idx) const -> bool
{
    return lua_isstring(_luaState, idx);
}

auto State::is_table(i32 idx) const -> bool
{
    return lua_istable(_luaState, idx);
}

auto State::is_thread(i32 idx) const -> bool
{
    return lua_isthread(_luaState, idx);
}

auto State::is_nil(i32 idx) const -> bool
{
    return lua_isnil(_luaState, idx);
}

auto State::is_userdata(i32 idx) const -> bool
{
    return lua_isuserdata(_luaState, idx);
}

auto State::to_bool(i32 idx) const -> bool
{
    return lua_toboolean(_luaState, idx);
}

auto State::to_integer(i32 idx) const -> i64
{
    return lua_tointeger(_luaState, idx);
}

auto State::to_number(i32 idx) const -> f64
{
    return lua_tonumber(_luaState, idx);
}

auto State::to_string(i32 idx) const -> const char*
{
    return lua_tostring(_luaState, idx);
}

auto State::to_userdata(i32 idx) const -> void*
{
    return lua_touserdata(_luaState, idx);
}

auto State::to_thread(i32 idx) const -> State
{
    return State { lua_tothread(_luaState, idx) };
}

auto State::get_type(i32 idx) const -> Type
{
    return convert_type(lua_type(_luaState, idx));
}

auto State::get_top() const -> i32
{
    return lua_gettop(_luaState);
}

auto State::check_stack(i32 size) const -> bool
{
    return lua_checkstack(_luaState, size);
}

auto State::resume(i32 argCount, i32* resultCount) const -> ThreadState
{
    const i32 err { lua_resume(_luaState, nullptr, argCount, resultCount) };
    switch (err) {
    case LUA_OK:
        return ThreadState::Ok;
    case LUA_YIELD:
        return ThreadState::Yielded;
    case LUA_ERRRUN:
        return ThreadState::RuntimeError;
    case LUA_ERRSYNTAX:
        return ThreadState::SyntaxError;
    case LUA_ERRMEM:
        return ThreadState::MemError;
    case LUA_ERRERR:
        return ThreadState::ErrorError;
    }

    return ThreadState::ErrorError;
}

auto State::next(i32 idx) const -> bool
{
    return lua_next(_luaState, idx) != 0;
}

void State::push_bool(bool val) const
{
    lua_pushboolean(_luaState, val);
}

void State::push_cfunction(i32 (*lua_CFunction)(lua_State*)) const
{
    lua_pushcfunction(_luaState, lua_CFunction);
}

void State::push_cclosure(i32 (*lua_CFunction)(lua_State*), i32 n) const
{
    lua_pushcclosure(_luaState, lua_CFunction, n);
}

void State::push_integer(i64 val) const
{
    lua_pushinteger(_luaState, val);
}

void State::push_lightuserdata(void* p) const
{
    lua_pushlightuserdata(_luaState, p);
}

void State::push_nil() const
{
    lua_pushnil(_luaState);
}

void State::push_number(f64 val) const
{
    lua_pushnumber(_luaState, val);
}

void State::push_string(const char* val) const
{
    lua_pushstring(_luaState, val);
}

void State::push_value(i32 idx) const
{
    lua_pushvalue(_luaState, idx);
}

void State::pop(i32 count) const
{
    lua_pop(_luaState, count);
}

void State::remove(i32 count) const
{
    lua_remove(_luaState, count);
}

auto State::get_table(i32 idx) const -> Type
{
    return convert_type(lua_gettable(_luaState, idx));
}

void State::get_metatable(const char* tableName) const
{
    luaL_getmetatable(_luaState, tableName);
}

void State::set_table(i32 idx) const
{
    lua_settable(_luaState, idx);
}

void State::set_metatable(i32 idx) const
{
    lua_setmetatable(_luaState, idx);
}

void State::new_table() const
{
    lua_newtable(_luaState);
}

void State::create_table(i32 narr, i32 nrec) const
{
    lua_createtable(_luaState, narr, nrec);
}

auto State::new_metatable(const char* tableName) const -> i32
{
    return luaL_newmetatable(_luaState, tableName);
}

auto State::new_userdata(i32 size) const -> void*
{
    return lua_newuserdatauv(_luaState, size, 0);
}

auto State::new_userdata(i32 size, i32 nuvalue) const -> void*
{
    return lua_newuserdatauv(_luaState, size, nuvalue);
}

void State::set_registry_field(const char* name) const
{
    lua_setfield(_luaState, LUA_REGISTRYINDEX, name);
}

void State::insert(i32 idx) const
{
    lua_insert(_luaState, idx);
}

auto State::raw_len(i32 idx) const -> u64
{
    return lua_rawlen(_luaState, idx);
}

auto State::raw_get(i32 idx, i64 n) const -> Type
{
    return convert_type(lua_rawgeti(_luaState, idx, n));
}

auto State::raw_get(i32 idx) const -> Type
{
    return convert_type(lua_rawget(_luaState, idx));
}

void State::raw_set(i32 idx, i64 n) const
{
    lua_rawseti(_luaState, idx, n);
}

void State::raw_set(i32 idx) const
{
    lua_rawset(_luaState, idx);
}

auto State::get_uservalue(i32 index, i32 n) const -> i32
{
    return lua_getiuservalue(_luaState, index, n);
}

auto State::set_uservalue(i32 index, i32 n) const -> i32
{
    return lua_setiuservalue(_luaState, index, n);
}

auto State::UpvalueIndex(i32 n) -> i32
{
    return lua_upvalueindex(n);
}

auto State::lua() const -> lua_State*
{
    return _luaState;
}

auto State::create_stack_guard() const -> StackGuard
{
    return StackGuard { _luaState };
}

auto State::ref(i32 idx) const -> i32
{
    return luaL_ref(_luaState, idx);
}

void State::unref(i32 t, i32 ref) const
{
    luaL_unref(_luaState, t, ref);
}

auto State::status() const -> i32
{
    return lua_status(_luaState);
}

auto State::reset_thread() const -> i32
{
    return lua_resetthread(_luaState);
}

auto State::do_call(i32 nargs, i32 nret) const -> ResultState
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
        return ResultState::RuntimeError;
    case LUA_ERRMEM:
        return ResultState::MemAllocError;
    case LUA_OK:
        return ResultState::Ok;
    default:
        return ResultState::RuntimeError;
    }
}

}
