// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/scripting/lua/Lua.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

    #include <lauxlib.h>
    #include <lua.h>
    #include <lualib.h>

    #include "tcob/core/FlatMap.hpp"
    #include "tcob/core/Logger.hpp"

namespace tcob::scripting::lua {

////////////////////////////////////////////////////////////

stack_guard::stack_guard(lua_State* l)
    : _luaState {l}
    , _oldTop {lua_gettop(l)}
{
}

stack_guard::~stack_guard()
{
    i32 const n {lua_gettop(_luaState) - _oldTop};
    if (n > 0) {
        lua_pop(_luaState, n);
    }
}

auto stack_guard::get_top() const -> i32
{
    return _oldTop;
}

////////////////////////////////////////////////////////////
static flat_map<i32, type> const typeMap {
    {LUA_TNONE, type::None},
    {LUA_TNIL, type::Nil},
    {LUA_TBOOLEAN, type::Boolean},
    {LUA_TLIGHTUSERDATA, type::LightUserdata},
    {LUA_TNUMBER, type::Number},
    {LUA_TSTRING, type::String},
    {LUA_TTABLE, type::Table},
    {LUA_TFUNCTION, type::Function},
    {LUA_TUSERDATA, type::Userdata},
    {LUA_TTHREAD, type::Thread}};

////////////////////////////////////////////////////////////

state_view::state_view(lua_State* l)
    : _view {l}
{
}

auto state_view::is_bool(i32 idx) const -> bool
{
    return lua_isboolean(_view, idx);
}

auto state_view::is_function(i32 idx) const -> bool
{
    return lua_isfunction(_view, idx);
}

auto state_view::is_integer(i32 idx) const -> bool
{
    return lua_isinteger(_view, idx) != 0;
}

auto state_view::is_number(i32 idx) const -> bool
{
    return lua_isnumber(_view, idx) != 0;
}

auto state_view::is_string(i32 idx) const -> bool
{
    return lua_isstring(_view, idx) != 0;
}

auto state_view::is_table(i32 idx) const -> bool
{
    return lua_istable(_view, idx);
}

auto state_view::is_thread(i32 idx) const -> bool
{
    return lua_isthread(_view, idx);
}

auto state_view::is_nil(i32 idx) const -> bool
{
    return lua_isnil(_view, idx);
}

auto state_view::is_none(i32 idx) const -> bool
{
    return lua_isnone(_view, idx);
}

auto state_view::is_none_or_nil(i32 idx) const -> bool
{
    return lua_isnoneornil(_view, idx);
}

auto state_view::is_userdata(i32 idx) const -> bool
{
    return lua_isuserdata(_view, idx) != 0;
}

auto state_view::to_bool(i32 idx) const -> bool
{
    return lua_toboolean(_view, idx) != 0;
}

auto state_view::to_integer(i32 idx) const -> i64
{
    return lua_tointeger(_view, idx);
}

auto state_view::to_number(i32 idx) const -> f64
{
    return lua_tonumber(_view, idx);
}

auto state_view::to_string(i32 idx) const -> char const*
{
    return lua_tostring(_view, idx);
}

auto state_view::to_userdata(i32 idx) const -> void*
{
    return lua_touserdata(_view, idx);
}

auto state_view::to_thread(i32 idx) const -> state_view
{
    return state_view {lua_tothread(_view, idx)};
}

auto state_view::get_type(i32 idx) const -> type
{
    return typeMap.at(lua_type(_view, idx));
}

auto state_view::get_top() const -> i32
{
    return lua_gettop(_view);
}

auto state_view::check_stack(i32 size) const -> bool
{
    return lua_checkstack(_view, size) != 0;
}

auto state_view::resume(i32 argCount, i32* resultCount) const -> coroutine_status
{
    i32 const err {lua_resume(_view, nullptr, argCount, resultCount)};
    switch (err) {
    case LUA_OK:
        return coroutine_status::Dead;
    case LUA_YIELD:
        return coroutine_status::Suspended;
    case LUA_ERRRUN:
        return coroutine_status::RuntimeError;
    case LUA_ERRSYNTAX:
        return coroutine_status::SyntaxError;
    case LUA_ERRMEM:
        return coroutine_status::MemError;
    case LUA_ERRERR:
        return coroutine_status::ErrorError;
    }

    return coroutine_status::ErrorError;
}

auto state_view::next(i32 idx) const -> bool
{
    return lua_next(_view, idx) != 0;
}

void state_view::push_bool(bool val) const
{
    lua_pushboolean(_view, static_cast<i32>(val));
}

void state_view::push_cfunction(i32 (*lua_CFunction)(lua_State*)) const
{
    lua_pushcfunction(_view, lua_CFunction);
}

void state_view::push_cclosure(i32 (*lua_CFunction)(lua_State*), i32 n) const
{
    lua_pushcclosure(_view, lua_CFunction, n);
}

void state_view::push_integer(i64 val) const
{
    lua_pushinteger(_view, val);
}

void state_view::push_lightuserdata(void* p) const
{
    lua_pushlightuserdata(_view, p);
}

void state_view::push_nil() const
{
    lua_pushnil(_view);
}

void state_view::push_number(f64 val) const
{
    lua_pushnumber(_view, val);
}

void state_view::push_string(string const& val) const
{
    lua_pushstring(_view, val.c_str());
}

void state_view::push_lstring(string_view val) const
{
    lua_pushlstring(_view, val.data(), val.size());
}

void state_view::push_value(i32 idx) const
{
    lua_pushvalue(_view, idx);
}

void state_view::pop(i32 count) const
{
    lua_pop(_view, count);
}

void state_view::remove(i32 idx) const
{
    lua_remove(_view, idx);
}

auto state_view::get_table(i32 idx) const -> type
{
    return typeMap.at(lua_gettable(_view, idx));
}

auto state_view::get_metatable(i32 objindex) const -> i32
{
    return lua_getmetatable(_view, objindex);
}

void state_view::get_metatable(string const& tableName) const
{
    luaL_getmetatable(_view, tableName.c_str());
}

void state_view::set_table(i32 idx) const
{
    lua_settable(_view, idx);
}

void state_view::set_metatable(i32 idx) const
{
    lua_setmetatable(_view, idx);
}

void state_view::new_table() const
{
    lua_newtable(_view);
}

void state_view::create_table(i32 narr, i32 nrec) const
{
    lua_createtable(_view, narr, nrec);
}

auto state_view::new_metatable(string const& tableName) const -> i32
{
    return luaL_newmetatable(_view, tableName.c_str());
}

auto state_view::new_userdata(usize size) const -> void*
{
    return lua_newuserdatauv(_view, size, 0);
}

auto state_view::new_userdata(usize size, i32 nuvalue) const -> void*
{
    return lua_newuserdatauv(_view, size, nuvalue);
}

void state_view::set_registry_field(string const& name) const
{
    lua_setfield(_view, LUA_REGISTRYINDEX, name.c_str());
}

void state_view::insert(i32 idx) const
{
    lua_insert(_view, idx);
}

auto state_view::raw_len(i32 idx) const -> u64
{
    return lua_rawlen(_view, idx);
}

auto state_view::raw_get(i32 idx, i64 n) const -> type
{
    return typeMap.at(lua_rawgeti(_view, idx, n));
}

auto state_view::raw_get(i32 idx) const -> type
{
    return typeMap.at(lua_rawget(_view, idx));
}

void state_view::raw_set(i32 idx, i64 n) const
{
    lua_rawseti(_view, idx, n);
}

void state_view::raw_set(i32 idx) const
{
    lua_rawset(_view, idx);
}

auto state_view::get_uservalue(i32 index, i32 n) const -> i32
{
    return lua_getiuservalue(_view, index, n);
}

auto state_view::set_uservalue(i32 index, i32 n) const -> i32
{
    return lua_setiuservalue(_view, index, n);
}

auto state_view::GetUpvalueIndex(i32 n) -> i32
{
    return lua_upvalueindex(n);
}

auto state_view::create_stack_guard() const -> stack_guard
{
    return stack_guard {_view};
}

auto state_view::ref(i32 idx) const -> i32
{
    return luaL_ref(_view, idx);
}

void state_view::unref(i32 t, i32 ref) const
{
    luaL_unref(_view, t, ref);
}

auto state_view::is_yieldable() const -> bool
{
    return lua_isyieldable(_view) != 0;
}

auto state_view::status() const -> i32
{
    return lua_status(_view);
}

auto state_view::close_thread() const -> i32
{
    return lua_closethread(_view, nullptr);
}

auto state_view::close_thread(state_view from) const -> i32
{
    return lua_closethread(_view, from._view);
}

void state_view::error(string const& message) const
{
    luaL_error(_view, message.c_str());
}

auto static error_handler(lua_State* l) -> i32
{
    i32 const n {lua_gettop(l)};
    if (lua_isstring(l, n) != 0) {
        logger::Error("Lua: {}", lua_tostring(l, n));
    }
    return 0;
}

auto state_view::pcall(i32 nargs, i32 nret) const -> error_code
{
    i32 const hpos {get_top() - nargs};

    push_cfunction(&error_handler);
    insert(hpos);
    i32 const err {lua_pcall(_view, nargs, nret, hpos)};
    remove(hpos);

    switch (err) {
    case LUA_OK:
        return error_code::Ok;
    case LUA_ERRRUN:
    case LUA_ERRMEM:
    default:
        return error_code::Error;
    }
}

void state_view::requiref(string const& modname, lua_CFunction openf, bool glb) const
{
    luaL_requiref(_view, modname.c_str(), openf, static_cast<i32>(glb));
}

auto state_view::load_buffer(string_view script, string const& name) const -> i32
{
    return luaL_loadbuffer(_view, script.data(), script.size(), name.c_str());
}

auto state_view::load_buffer(string_view script, string const& name, string const& mode) const -> i32
{
    return luaL_loadbufferx(_view, script.data(), script.size(), name.c_str(), mode.c_str());
}

void state_view::set_warnf(lua_WarnFunction f, void* ud) const
{
    lua_setwarnf(_view, f, ud);
}

void state_view::set_hook(lua_Hook func, i32 mask, i32 count) const
{
    lua_sethook(_view, func, mask, count);
}

auto state_view::gc(i32 what, i32 a, i32 b, i32 c) const -> i32
{
    return lua_gc(_view, what, a, b, c);
}

auto state_view::dump(lua_Writer writer, void* data, i32 strip) const -> i32
{
    return lua_dump(_view, writer, data, strip);
}

auto state_view::get_upvalue(i32 funcindex, i32 n) const -> char const*
{
    return lua_getupvalue(_view, funcindex, n);
}

auto state_view::set_upvalue(i32 funcindex, i32 n) const -> char const*
{
    return lua_setupvalue(_view, funcindex, n);
}

void state_view::push_globaltable() const
{
    lua_pushglobaltable(_view);
}

auto state_view::traceback(i32 level) const -> string
{
    luaL_traceback(_view, _view, nullptr, level);
    return to_string(-1);
}

auto state_view::raw_equal(i32 idx1, i32 idx2) const -> bool
{
    return lua_rawequal(_view, idx1, idx2) == 1;
}

auto state_view::NewState() -> lua_State*
{
    return luaL_newstate();
}

void state_view::close() const
{
    if (_view != nullptr) {
        lua_close(_view);
    }
}

auto state_view::is_valid() const -> bool
{
    return _view != nullptr;
}

////////////////////////////////////////////////////////////

}

#endif