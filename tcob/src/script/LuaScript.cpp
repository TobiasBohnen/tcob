// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/script/LuaScript.hpp>

#include <lua.hpp>

namespace tcob::lua {
static const std::unordered_map<Library, std::pair<const char*, lua_CFunction>> Libraries = {
    { Library::Table, { LUA_TABLIBNAME, luaopen_table } },
    { Library::String, { LUA_STRLIBNAME, luaopen_string } },
    { Library::Math, { LUA_MATHLIBNAME, luaopen_math } },
    { Library::Coroutine, { LUA_COLIBNAME, luaopen_coroutine } },
    { Library::IO, { LUA_IOLIBNAME, luaopen_io } },
    { Library::OS, { LUA_OSLIBNAME, luaopen_os } },
    { Library::Utf8, { LUA_UTF8LIBNAME, luaopen_utf8 } },
    { Library::Debug, { LUA_DBLIBNAME, luaopen_debug } },
    { Library::Package, { LUA_LOADLIBNAME, luaopen_package } }
};

Script::Script()
    : _state { luaL_newstate() }
{
    lua_State* ls { _state.lua() };
    luaL_requiref(ls, "", luaopen_base, 1);
    _state.pop(1);

    lua_pushglobaltable(ls);
    _globalTable = std::make_unique<Table>(_state, -1);
    _state.pop(1);
}

Script::~Script()
{
    _wrappers.clear();
    _globalTable = nullptr;
    lua_close(_state.lua());
}

auto Script::global_table() const -> const Table&
{
    return *_globalTable;
}

void Script::perform_GC() const
{
    lua_gc(_state.lua(), LUA_GCCOLLECT, 0);
}

void Script::stop_GC() const
{
    lua_gc(_state.lua(), LUA_GCSTOP, 0);
}

void Script::restart_GC() const
{
    lua_gc(_state.lua(), LUA_GCRESTART, 0);
}

auto Script::do_call(i32 nargs, i32 nret) const -> ResultState
{
    return _state.do_call(nargs, nret);
}

auto Script::call_buffer(const byte* script, isize length, const std::string& name) const -> ResultState
{
    const i32 err { luaL_loadbuffer(_state.lua(), script, length, name.c_str()) };
    if (err == LUA_OK) {
        return do_call(0, LUA_MULTRET);
    } else {
        const i32 n { _state.get_top() };
        Log("Lua says: " + std::string(_state.to_string(n)), LogLevel::Error);

        switch (err) {
        case LUA_ERRSYNTAX:
            return ResultState::SyntaxError;
        case LUA_ERRMEM:
            return ResultState::MemAllocError;
        default:
            return ResultState::RuntimeError;
        }
    }
}

auto Script::load_binarybuffer(const std::string& file) const -> bool
{
    std::string s { FileSystem::read_as_string(file) };
    return luaL_loadbufferx(_state.lua(), s.c_str(), s.length(), file.c_str(), "b") == LUA_OK;
}

void Script::load_library(Library lib)
{
    const auto& [name, func] = Libraries.at(lib);
    luaL_requiref(_state.lua(), name, func, 1);
    _state.pop(1);
}

void Script::register_searcher(const std::function<Table(Script&, const std::string&)>& func)
{
    _loader = [this, func](const std::string& name) -> Table {
        return func(*this, name);
    };
    auto searcher { [this](const std::string&) -> std::function<Table(const std::string&)>& {
        return _loader;
    } };
    _searcher = std::function<std::function<Table(const std::string&)>&(const std::string&)>(searcher);

    Table searchers { global_table()["package"]["searchers"] };
    searchers[5] = _searcher;
}
}