// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/scripting/Script.hpp"

#include <memory>
#include <optional>
#include <utility>

#include "tcob/core/Logger.hpp"
#include "tcob/scripting/Lua.hpp"
#include "tcob/scripting/Scripting.hpp"
#include "tcob/scripting/Types.hpp"

namespace tcob::scripting {

static void warn(void* ud, char const* msg, int toCont)
{
    auto* scr {static_cast<script*>(ud)};
    scr->Warning({.Message = msg, .ToCont = toCont != 0});
}

script::script()
    : _view {state_view::NewState()}
{
    _view.require_library(library::Base);
    _view.pop(1);

    _view.push_globaltable();
    _globalTable.acquire(_view, -1);
    _view.pop(1);

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

auto script::view() const -> state_view
{
    return _view;
}

auto script::gc() const -> garbage_collector
{
    return garbage_collector {_view};
}

auto script::create_table() const -> table
{
    return table::Create(_view);
}

auto script::call_buffer(string_view script, string const& name) const -> std::optional<error_code>
{
    if (_view.load_buffer(script, name)) {
        if (_environment) {
            function<void> func {function<void>::Acquire(_view, -1)};
            if (!func.set_environment(*_environment)) {
                return error_code::Error;
            }
        }
        return _view.pcall(0);
    }

    logger::Error("Lua: {}", _view.to_string(_view.get_top()));

    return error_code::Error;
}

auto script::load_binary_buffer(string_view script, string const& name) const -> bool
{
    return _view.load_buffer(script, name, "b");
}

void script::load_library(library lib)
{
    _view.require_library(lib);
    _view.pop(1);

    if (lib == library::Package) { register_searcher(); }
}

void script::register_searcher()
{
    if (!_globalTable.has("package", "searchers")) { return; }

    _loader = [this](string const& name) -> table {
        require_event ev {.Name = name, .Table = std::nullopt};
        Require(ev);
        return ev.Table.has_value() ? *ev.Table : run_file<table>(name + ".lua").value();
    };

    _searcher = [this](string const&) -> LoaderFunc* { return &_loader; };

    table tab {_globalTable["package"]["searchers"].as<table>()};
    tab[tab.raw_length() + 1] = &_searcher;
}

static void hook(lua_State* l, lua_Debug* ar)
{
    state_view ls {l};
    auto const guard {ls.create_scoped_stack()};

    ls.get_metatable("_tcob");
    table lt {table::Acquire(ls, -1)};
    if (lt.has("_hook")) {
        ls.get_info(ar);
        debug dbg {&ls, ar};

        auto* hook {reinterpret_cast<script::HookFunc*>(lt["_hook"].as<void*>())};
        (*hook)(dbg);
    }
}

void script::set_hook(HookFunc&& func, debug_mask mask)
{
    auto const guard {_view.create_scoped_stack()};

    _view.new_metatable("_tcob");
    i32 const tableIdx {_view.get_top()};

    _view.push_convert("_hook");
    _hookFunc = std::move(func);
    _view.push_convert(reinterpret_cast<void*>(&_hookFunc));
    _view.set_table(tableIdx);
    _view.set_hook(&hook, debug::GetMask(mask), 1);
}

void script::remove_hook()
{
    _view.set_hook(nullptr, 0, 0);
}

void script::raise_error(string const& message)
{
    _view.error(message);
}

void script::clear_wrappers()
{
    _wrappers.clear();
}

}

////////////////////////////////////////////////////////////

auto tcob::literals::operator""_lua(char const* str, usize) -> std::unique_ptr<tcob::scripting::script>
{
    auto retValue {std::make_unique<tcob::scripting::script>()};
    (void)retValue->run(string {str});
    return retValue;
}
