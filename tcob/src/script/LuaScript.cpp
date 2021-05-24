// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/script/LuaScript.hpp>

#include <lua.hpp>

#include <tcob/core/io/Logger.hpp>

namespace tcob {
static const std::unordered_map<LuaLibrary, std::pair<const char*, lua_CFunction>> Libraries = {
    { LuaLibrary::Table, { LUA_TABLIBNAME, luaopen_table } },
    { LuaLibrary::String, { LUA_STRLIBNAME, luaopen_string } },
    { LuaLibrary::Math, { LUA_MATHLIBNAME, luaopen_math } },
    { LuaLibrary::Coroutine, { LUA_COLIBNAME, luaopen_coroutine } },
    { LuaLibrary::IO, { LUA_IOLIBNAME, luaopen_io } },
    { LuaLibrary::OS, { LUA_OSLIBNAME, luaopen_os } },
    { LuaLibrary::Utf8, { LUA_UTF8LIBNAME, luaopen_utf8 } },
    { LuaLibrary::Debug, { LUA_DBLIBNAME, luaopen_debug } },
    { LuaLibrary::Package, { LUA_LOADLIBNAME, luaopen_package } }
};

LuaScript::LuaScript()
    : _state { luaL_newstate() }
{
    lua_State* ls { _state.lua() };
    luaL_requiref(ls, "", luaopen_base, 1);
    _state.pop(1);

    lua_pushglobaltable(ls);
    _globalTable = new LuaTable { ls, -1 };
    _state.pop(1);
}

LuaScript::~LuaScript()
{
    _wrappers.clear();
    delete _globalTable;
    lua_close(_state.lua());
}

auto LuaScript::global_table() const -> const LuaTable&
{
    return *_globalTable;
}

void LuaScript::perform_GC() const
{
    lua_gc(_state.lua(), LUA_GCCOLLECT, 0);
}

void LuaScript::stop_GC() const
{
    lua_gc(_state.lua(), LUA_GCSTOP, 0);
}

void LuaScript::restart_GC() const
{
    lua_gc(_state.lua(), LUA_GCRESTART, 0);
}

auto LuaScript::do_call(i32 nargs, i32 nret) const -> LuaResultState
{
    const i32 hpos { _state.get_top() - nargs };
    i32 err { 0 };

    _state.push_cfunction(
        [](lua_State* l) -> i32 {
        const i32 n { lua_gettop(l)};
        Log("Lua says: " + std::string(lua_tostring(l, n)));
        return 0; });
    _state.insert(hpos);
    err = lua_pcall(_state.lua(), nargs, nret, hpos);
    _state.remove(hpos);

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

auto LuaScript::call_buffer(const byte* script, isize length, const std::string& name) const -> LuaResultState
{
    const i32 err { luaL_loadbuffer(_state.lua(), script, length, name.c_str()) };
    if (err == LUA_OK) {
        return do_call(0, LUA_MULTRET);
    } else {
        const i32 n { _state.get_top() };
        Log("Lua says: " + std::string(_state.to_string(n)));

        switch (err) {
        case LUA_ERRSYNTAX:
            return LuaResultState::SyntaxError;
        case LUA_ERRMEM:
            return LuaResultState::MemAllocError;
        default:
            return LuaResultState::RuntimeError;
        }
    }
}

auto LuaScript::load_binarybuffer(const std::string& file) const -> bool
{
    std::string s { FileSystem::read_as_string(file) };
    return luaL_loadbufferx(_state.lua(), s.c_str(), s.length(), file.c_str(), "b") == LUA_OK;
}

void LuaScript::load_library(LuaLibrary lib)
{
    const auto& [name, func] = Libraries.at(lib);
    luaL_requiref(_state.lua(), name, func, 1);
    _state.pop(1);
}

void LuaScript::register_searcher(const std::function<LuaTable(LuaScript&, const std::string&)>& func)
{
    _loader = [this, func](const std::string& name) -> LuaTable {
        return func(*this, name);
    };
    auto searcher { [this](const std::string&) -> std::function<LuaTable(const std::string&)>& {
        return _loader;
    } };
    _searcher = std::function<std::function<LuaTable(const std::string&)>&(const std::string&)>(searcher);

    LuaTable searchers { global_table()["package"]["searchers"] };
    searchers[5] = _searcher;
}
}