// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/scripting/Lua.hpp"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <optional>
#include <unordered_map>
#include <utility>

#include "tcob/core/Logger.hpp"
#include "tcob/scripting/Scripting.hpp"

namespace tcob::scripting {

static_assert(NOREF == LUA_NOREF);
static_assert(REGISTRYINDEX == LUA_REGISTRYINDEX);

////////////////////////////////////////////////////////////

debug::debug(state_view* view, lua_Debug* ar)
    : Event {static_cast<debug_event>(ar->event)}
    , Name {ar->name != nullptr ? ar->name : ""}
    , What {ar->what != nullptr ? ar->what : ""}
    , Source {ar->source != nullptr ? ar->source : ""}
    , CurrentLine {ar->currentline}
    , LineDefined {ar->linedefined}
    , LastLineDefined {ar->lastlinedefined}
    , NameWhat {ar->namewhat != nullptr ? ar->namewhat : ""}
    , UpvalueCount {ar->nups}
    , ParameterCount {ar->nparams}
    , IsVarArg {ar->isvararg != 0}
    , IsTailCall {ar->istailcall != 0}
    , FirstTransfer {ar->ftransfer}
    , TransferredValueCount {ar->ntransfer}
    , ShortSource {ar->short_src}
    , _view {view}
    , _ar {ar}
{
}

auto debug::get_local(i32 n) const -> string
{
    return _view->get_local(_ar, n);
}

auto debug::set_local(i32 n) const -> string
{
    return _view->set_local(_ar, n);
}

auto debug::GetMask(debug_mask mask) -> i32
{
    i32 retValue {0};
    if (mask.Call) { retValue |= LUA_MASKCALL; }
    if (mask.Return) { retValue |= LUA_MASKRET; }
    if (mask.Line) { retValue |= LUA_MASKLINE; }
    if (mask.Count) { retValue |= LUA_MASKCOUNT; }
    return retValue;
}

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

static auto GetType(i32 i) -> type
{
    switch (i) {
    case LUA_TNONE:          return type::None;
    case LUA_TNIL:           return type::Nil;
    case LUA_TBOOLEAN:       return type::Boolean;
    case LUA_TLIGHTUSERDATA: return type::LightUserdata;
    case LUA_TNUMBER:        return type::Number;
    case LUA_TSTRING:        return type::String;
    case LUA_TTABLE:         return type::Table;
    case LUA_TFUNCTION:      return type::Function;
    case LUA_TUSERDATA:      return type::Userdata;
    case LUA_TTHREAD:        return type::Thread;
    }

    return type::None;
}

////////////////////////////////////////////////////////////

state_view::state_view(lua_State* l)
    : _state {l}
{
}

auto state_view::is_bool(i32 idx) const -> bool
{
    return lua_isboolean(_state, idx);
}

auto state_view::is_function(i32 idx) const -> bool
{
    return lua_isfunction(_state, idx);
}

auto state_view::is_integer(i32 idx) const -> bool
{
    return lua_isinteger(_state, idx) != 0;
}

auto state_view::is_number(i32 idx) const -> bool
{
    return lua_isnumber(_state, idx) != 0;
}

auto state_view::is_string(i32 idx) const -> bool
{
    return lua_isstring(_state, idx) != 0;
}

auto state_view::is_table(i32 idx) const -> bool
{
    return lua_istable(_state, idx);
}

auto state_view::is_thread(i32 idx) const -> bool
{
    return lua_isthread(_state, idx);
}

auto state_view::is_nil(i32 idx) const -> bool
{
    return lua_isnil(_state, idx);
}

auto state_view::is_none(i32 idx) const -> bool
{
    return lua_isnone(_state, idx);
}

auto state_view::is_none_or_nil(i32 idx) const -> bool
{
    return lua_isnoneornil(_state, idx);
}

auto state_view::is_userdata(i32 idx) const -> bool
{
    return lua_isuserdata(_state, idx) != 0;
}

auto state_view::to_bool(i32 idx) const -> bool
{
    return lua_toboolean(_state, idx) != 0;
}

auto state_view::to_integer(i32 idx) const -> i64
{
    return lua_tointeger(_state, idx);
}

auto state_view::to_number(i32 idx) const -> f64
{
    return lua_tonumber(_state, idx);
}

auto state_view::to_string(i32 idx) const -> char const*
{
    return lua_tostring(_state, idx);
}

auto state_view::to_userdata(i32 idx) const -> void*
{
    return lua_touserdata(_state, idx);
}

auto state_view::to_thread(i32 idx) const -> state_view
{
    return state_view {lua_tothread(_state, idx)};
}

auto state_view::get_type(i32 idx) const -> type
{
    return GetType(lua_type(_state, idx));
}

auto state_view::get_top() const -> i32
{
    return lua_gettop(_state);
}

auto state_view::info(string const& what, lua_Debug* ar) const -> bool
{
    return lua_getinfo(_state, what.c_str(), ar) != 0;
}

auto state_view::get_local(lua_Debug* ar, i32 n) const -> string
{
    auto const* r {lua_getlocal(_state, ar, n)};
    return r == nullptr ? "" : r;
}

auto state_view::set_local(lua_Debug* ar, i32 n) const -> string
{
    auto const* r {lua_setlocal(_state, ar, n)};
    return r == nullptr ? "" : r;
}

auto state_view::check_stack(i32 size) const -> bool
{
    return lua_checkstack(_state, size) != 0;
}

auto state_view::resume(i32 argCount) const -> coroutine_status
{
    i32       resultCount {0};
    i32 const err {lua_resume(_state, nullptr, argCount, &resultCount)};

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
        return coroutine_status::Error;
    }

    return coroutine_status::Error;
}

auto state_view::next(i32 idx) const -> bool
{
    return lua_next(_state, idx) != 0;
}

void state_view::push_bool(bool val) const
{
    lua_pushboolean(_state, static_cast<i32>(val));
}

void state_view::push_cfunction(i32 (*lua_CFunction)(lua_State*)) const
{
    lua_pushcfunction(_state, lua_CFunction);
}

void state_view::push_cclosure(i32 (*lua_CFunction)(lua_State*), i32 n) const
{
    lua_pushcclosure(_state, lua_CFunction, n);
}

void state_view::push_integer(i64 val) const
{
    lua_pushinteger(_state, val);
}

void state_view::push_lightuserdata(void* p) const
{
    lua_pushlightuserdata(_state, p);
}

void state_view::push_nil() const
{
    lua_pushnil(_state);
}

void state_view::push_number(f64 val) const
{
    lua_pushnumber(_state, val);
}

void state_view::push_string(string const& val) const
{
    lua_pushstring(_state, val.c_str());
}

void state_view::push_string(char const* val) const
{
    lua_pushstring(_state, val);
}

void state_view::push_lstring(string_view val) const
{
    lua_pushlstring(_state, val.data(), val.size());
}

void state_view::push_value(i32 idx) const
{
    lua_pushvalue(_state, idx);
}

void state_view::pop(i32 count) const
{
    lua_pop(_state, count);
}

void state_view::remove(i32 idx) const
{
    lua_remove(_state, idx);
}

auto state_view::get_table(i32 idx) const -> type
{
    return GetType(lua_gettable(_state, idx));
}

auto state_view::get_metatable(i32 objindex) const -> bool
{
    return lua_getmetatable(_state, objindex) == 1;
}

void state_view::get_metatable(string const& tableName) const
{
    luaL_getmetatable(_state, tableName.c_str());
}

void state_view::get_metatable(char const* tableName) const
{
    luaL_getmetatable(_state, tableName);
}

void state_view::set_table(i32 idx) const
{
    lua_settable(_state, idx);
}

void state_view::set_metatable(i32 idx) const
{
    lua_setmetatable(_state, idx);
}

void state_view::new_table() const
{
    lua_newtable(_state);
}

void state_view::create_table(i32 narr, i32 nrec) const
{
    lua_createtable(_state, narr, nrec);
}

auto state_view::new_metatable(string const& tableName) const -> i32
{
    return luaL_newmetatable(_state, tableName.c_str());
}

auto state_view::new_metatable(char const* tableName) const -> i32
{
    return luaL_newmetatable(_state, tableName);
}

auto state_view::new_userdata(usize size) const -> void*
{
    return lua_newuserdata(_state, size);
}

void state_view::set_field(i32 idx, string const& name) const
{
    lua_setfield(_state, idx, name.c_str());
}

void state_view::set_registry_field(string const& name) const
{
    lua_setfield(_state, LUA_REGISTRYINDEX, name.c_str());
}

void state_view::insert(i32 idx) const
{
    lua_insert(_state, idx);
}

auto state_view::raw_len(i32 idx) const -> u64
{
    return lua_rawlen(_state, idx);
}

auto state_view::raw_get(i32 idx, i64 n) const -> type
{
    return GetType(lua_rawgeti(_state, idx, n));
}

auto state_view::raw_get(i32 idx) const -> type
{
    return GetType(lua_rawget(_state, idx));
}

void state_view::raw_set(i32 idx, i64 n) const
{
    lua_rawseti(_state, idx, n);
}

void state_view::raw_set(i32 idx) const
{
    lua_rawset(_state, idx);
}

auto state_view::get_uservalue(i32 idx) const -> type
{
    return GetType(lua_getuservalue(_state, idx));
}

void state_view::get_field(i32 idx, string const& name) const
{
    lua_getfield(_state, idx, name.c_str());
}

auto state_view::set_uservalue(i32 idx) const -> i32
{
    return lua_setuservalue(_state, idx);
}

auto state_view::GetUpvalueIndex(i32 n) -> i32
{
    return lua_upvalueindex(n);
}

auto state_view::create_stack_guard() const -> stack_guard
{
    return stack_guard {_state};
}

auto state_view::ref(i32 idx) const -> i32
{
    return luaL_ref(_state, idx);
}

void state_view::unref(i32 t, i32 ref) const
{
    luaL_unref(_state, t, ref);
}

auto state_view::is_yieldable() const -> bool
{
    return lua_isyieldable(_state) != 0;
}

auto state_view::status() const -> i32
{
    return lua_status(_state);
}

auto state_view::close_thread() const -> bool
{
    return lua_closethread(_state, nullptr) == LUA_OK;
}

void state_view::error(string const& message) const
{
    luaL_error(_state, message.c_str());
}

void state_view::call(i32 nargs) const
{
    lua_call(_state, nargs, LUA_MULTRET);
}

static auto error_handler(lua_State* l) -> i32
{
    i32 const n {lua_gettop(l)};
    if (lua_isstring(l, n) != 0) {
        logger::Error("Lua: {}", lua_tostring(l, n));
    }
    return 0;
}

auto state_view::pcall(i32 nargs) const -> std::optional<error_code>
{
    i32 const hpos {get_top() - nargs};

    push_cfunction(&error_handler);
    insert(hpos);
    i32 const err {lua_pcall(_state, nargs, LUA_MULTRET, hpos)};
    remove(hpos);

    switch (err) {
    case LUA_OK:
        return std::nullopt;
    case LUA_ERRRUN:
    case LUA_ERRMEM:
    default:
        return error_code::Error;
    }
}

void state_view::requiref(string const& modname, lua_CFunction openf, bool glb) const
{
    luaL_requiref(_state, modname.c_str(), openf, static_cast<i32>(glb));
}

void state_view::require_library(library lib) const
{
    static std::unordered_map<library, std::pair<char const*, lua_CFunction>> const libraries {
        {library::Base, {"", luaopen_base}},
        {library::Table, {LUA_TABLIBNAME, luaopen_table}},
        {library::String, {LUA_STRLIBNAME, luaopen_string}},
        {library::Math, {LUA_MATHLIBNAME, luaopen_math}},
        {library::IO, {LUA_IOLIBNAME, luaopen_io}},
        {library::OS, {LUA_OSLIBNAME, luaopen_os}},
        {library::Debug, {LUA_DBLIBNAME, luaopen_debug}},
        {library::Package, {LUA_LOADLIBNAME, luaopen_package}},
        {library::Coroutine, {LUA_COLIBNAME, luaopen_coroutine}},
        {library::Utf8, {LUA_UTF8LIBNAME, luaopen_utf8}},
    };
    auto const& [name, func] {libraries.at(lib)};
    requiref(name, func, true);
}

auto state_view::load_buffer(string_view script, string const& name) const -> bool
{
    return luaL_loadbuffer(_state, script.data(), script.size(), name.c_str()) == LUA_OK;
}

auto state_view::load_buffer(string_view script, string const& name, string const& mode) const -> bool
{
    return luaL_loadbufferx(_state, script.data(), script.size(), name.c_str(), mode.c_str()) == LUA_OK;
}

void state_view::set_warnf([[maybe_unused]] lua_WarnFunction f, [[maybe_unused]] void* ud) const
{
    lua_setwarnf(_state, f, ud);
}

void state_view::set_hook(lua_Hook func, i32 mask, i32 count) const
{
    lua_sethook(_state, func, mask, count);
}

void state_view::get_info(lua_Debug* ar) const
{
    lua_getinfo(_state, "Slutnr", ar);
}

auto state_view::gc(i32 what, i32 a, [[maybe_unused]] i32 b, [[maybe_unused]] i32 c) const -> i32
{
    return lua_gc(_state, what, a, b, c);
}

auto state_view::dump(lua_Writer writer, void* data, [[maybe_unused]] i32 strip) const -> i32
{
    return lua_dump(_state, writer, data, strip);
}

auto state_view::get_upvalue(i32 funcindex, i32 n) const -> char const*
{
    return lua_getupvalue(_state, funcindex, n);
}

auto state_view::set_upvalue(i32 funcindex, i32 n) const -> char const*
{
    return lua_setupvalue(_state, funcindex, n);
}

void state_view::push_globaltable() const
{
    lua_pushglobaltable(_state);
}

auto state_view::traceback(i32 level) const -> string
{
    luaL_traceback(_state, _state, nullptr, level);
    return to_string(-1);
}

auto state_view::raw_equal(i32 idx1, i32 idx2) const -> bool
{
    return lua_rawequal(_state, idx1, idx2) == 1;
}

auto state_view::NewState() -> lua_State*
{
    return luaL_newstate();
}

void state_view::close()
{
    if (_state != nullptr) {
        lua_close(_state);
        _state = nullptr;
    }
}

auto state_view::is_valid() const -> bool
{
    return _state != nullptr;
}

////////////////////////////////////////////////////////////

garbage_collector::garbage_collector(state_view l)
    : _luaState {l}
{
}

void garbage_collector::start_incremental_mode([[maybe_unused]] i32 pause, [[maybe_unused]] i32 stepmul, [[maybe_unused]] i32 stepsize) const
{
    _luaState.gc(LUA_GCINC, pause, stepmul, stepsize);
}

void garbage_collector::start_generational_mode([[maybe_unused]] i32 minormul, [[maybe_unused]] i32 majormul) const
{
    _luaState.gc(LUA_GCGEN, minormul, majormul, 0);
}

void garbage_collector::collect() const
{
    _luaState.gc(LUA_GCCOLLECT, 0, 0, 0);
}

void garbage_collector::stop() const
{
    _luaState.gc(LUA_GCSTOP, 0, 0, 0);
}

void garbage_collector::restart() const
{
    _luaState.gc(LUA_GCRESTART, 0, 0, 0);
}

auto garbage_collector::is_running() const -> bool
{
    return _luaState.gc(LUA_GCISRUNNING, 0, 0, 0) != 0;
}

auto garbage_collector::count() const -> i32
{
    return _luaState.gc(LUA_GCCOUNT, 0, 0, 0);
}

////////////////////////////////////////////////////////////
}
