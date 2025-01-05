// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/scripting/lua/LuaScript.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

    #include <lua.h>
    #include <lualib.h>

    #include <unordered_map>

    #include "tcob/core/Logger.hpp"

namespace tcob::scripting::lua {

void static warn(void* ud, char const* msg, int toCont)
{
    auto* scr {static_cast<script*>(ud)};
    scr->Warning({msg, toCont != 0});
}

script::script()
    : _view {state_view::NewState()}
{
    _view.requiref("", luaopen_base, true);
    _view.pop(1);

    _view.push_globaltable();
    _globalTable.acquire(_view, -1);
    _view.pop(1);

    _environment = _globalTable; // NOLINT(cppcoreguidelines-prefer-member-initializer)

    _view.set_warnf(&warn, this);
}

script::~script()
{
    if (_environment) { _environment->release(); }
    _globalTable.release();
    clear_wrappers();
    _view.close();
}

auto script::global_table() -> table&
{
    return _globalTable;
}

auto script::get_environment() const -> std::optional<table>
{
    return _environment;
}

void script::set_environment(table const& env)
{
    _environment = env;
}

auto script::get_view() const -> state_view
{
    return _view;
}

auto script::gc() const -> garbage_collector
{
    return garbage_collector {_view};
}

auto script::create_table() const -> table
{
    return table {_view};
}

auto script::call_buffer(string_view script, string const& name) const -> error_code
{
    if (_view.load_buffer(script, name) == LUA_OK) {
        if (_environment) {
            function<void> func {function<void>::Acquire(_view, -1)};
            func.set_environment(*_environment);
        }
        return _view.pcall(0);
    }

    i32 const n {_view.get_top()};
    logger::Error("Lua: {}", _view.to_string(n));

    return error_code::Error;
}

auto script::load_binary_buffer(string_view script, string const& name) const -> bool
{
    return _view.load_buffer(script, name, "b") == LUA_OK;
}

void script::load_library(library lib)
{
    static std::unordered_map<library, std::pair<char const*, lua_CFunction>> const libraries {
        {library::Table, {LUA_TABLIBNAME, luaopen_table}},
        {library::String, {LUA_STRLIBNAME, luaopen_string}},
        {library::Math, {LUA_MATHLIBNAME, luaopen_math}},
        {library::Coroutine, {LUA_COLIBNAME, luaopen_coroutine}},
        {library::IO, {LUA_IOLIBNAME, luaopen_io}},
        {library::OS, {LUA_OSLIBNAME, luaopen_os}},
        {library::Utf8, {LUA_UTF8LIBNAME, luaopen_utf8}},
        {library::Debug, {LUA_DBLIBNAME, luaopen_debug}},
        {library::Package, {LUA_LOADLIBNAME, luaopen_package}}};

    auto const& [name, func] = libraries.at(lib);
    _view.requiref(name, func, true);
    _view.pop(1);

    if (lib == library::Package) {
        register_searcher();
    }
}

void script::register_searcher()
{
    if (!_globalTable.has("package", "searchers")) {
        return;
    }

    table searchers {_globalTable["package"]["searchers"].as<table>()};

    _loader = [&](string const& name) -> table {
        require_event ev {.Name = name, .Table = std::nullopt};
        Require(ev);
        return ev.Table.has_value() ? *ev.Table : run_file<table>(name + ".lua").value();
    };

    _searcher = [&](string const&) -> std::function<table(string const&)>* {
        return &_loader;
    };

    searchers[searchers.raw_length() + 1] = &_searcher;
}

void static hook(lua_State* l, lua_Debug* ar)
{
    state_view ls {l};
    auto const guard {ls.create_stack_guard()};

    ls.get_metatable("_tcob");
    table lt {table::Acquire(ls, -1)};
    if (lt.has("_hook")) {
        auto* hook {reinterpret_cast<std::function<void(debug const&)>*>(lt["_hook"].as<void*>())};
        lua_getinfo(l, "Slutnr", ar);
        debug dbg {&ls, ar};
        (*hook)(dbg);
    }
}

void script::set_hook(HookFunc&& func, debug_mask mask)
{
    auto const guard {_view.create_stack_guard()};

    _view.new_metatable("_tcob");
    i32 const tableIdx {_view.get_top()};
    _view.push_convert("_hook");
    _hookFunc = std::move(func);
    _view.push_convert(reinterpret_cast<void*>(&_hookFunc));
    _view.set_table(tableIdx);

    i32 dmask {0};
    if (mask.Call) { dmask |= LUA_MASKCALL; }
    if (mask.Return) { dmask |= LUA_MASKRET; }
    if (mask.Line) { dmask |= LUA_MASKLINE; }
    if (mask.Count) { dmask |= LUA_MASKCOUNT; }

    _view.set_hook(&hook, dmask, 1);
}

void script::remove_hook()
{
    _view.set_hook(nullptr, 0, 0);
}

void script::raise_error(string const& message)
{
    _view.error(message);
}

}

////////////////////////////////////////////////////////////

auto tcob::literals::operator""_lua(char const* str, usize) -> std::unique_ptr<tcob::scripting::lua::script>
{
    auto retValue {std::make_unique<tcob::scripting::lua::script>()};
    (void)retValue->run(string {str});
    return retValue;
}

#endif
