// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/scripting/LuaScript.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <utility>
#include <variant>

#include "tcob/core/Logger.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/scripting/Lua.hpp"
#include "tcob/scripting/LuaTypes.hpp"
#include "tcob/scripting/Scripting.hpp"

namespace tcob::scripting {

extern "C" {
static void Warn(void* ud, char const* msg, int toCont)
{
    auto* scr {static_cast<script*>(ud)};
    scr->Warning({.Message = msg, .ToCont = toCont != 0});
}

static void Hook(lua_State* l, lua_Debug* ar)
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
}

script::script()
    : _view {state_view::NewState()}
{
    _view.require_library(library::Base);
    _view.pop(1);

    _view.push_globaltable();
    _globalTable.acquire(_view, -1);
    _view.pop(1);

    _view.set_warnf(&Warn, this);
}

script::~script()
{
    if (*Environment) {
        Environment.mutate([](auto& env) { env->release(); });
    }
    _globalTable.release();
    clear_wrappers();
    _view.close();
}

auto script::global_table() -> table&
{
    return _globalTable;
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

void script::open_addons()
{
    auto const with {[&](string const& name, auto fn) {
        if (_globalTable.has(name)) { fn(_globalTable[name].as<table>()); }
    }};

    with("math", [](auto&& tab) {
        tab["clamp"]    = +[](f32 v, f32 low, f32 high) { return std::clamp(v, low, high); };
        tab["lerp"]     = +[](f32 a, f32 b, f32 t) { return a + ((b - a) * t); };
        tab["round"]    = +[](f32 v) { return std::round(v); };
        tab["sign"]     = +[](f32 v) -> f32 { return (v > 0.f) - (v < 0.f); };
        tab["saturate"] = +[](f32 v) { return std::clamp(v, 0.f, 1.f); };
        tab["wrap"]     = +[](f32 v, f32 low, f32 high) {
            f32 const range {high - low};
            f32       result {std::fmod(v - low, range)};
            if (result < 0.f) { result += range; }
            return low + result;
        };
    });

    with("string", [](auto&& tab) {
        tab["trim"]        = +[](string_view s) { return helper::trim(s); };
        tab["starts_with"] = +[](string_view s, string_view prefix) { return s.starts_with(prefix); };
        tab["ends_with"]   = +[](string_view s, string_view suffix) { return s.ends_with(suffix); };
        tab["split"]       = +[](string_view s, string_view delim) { return helper::split(s, delim); };
    });

    with("table", [](auto&& tab) {
        tab["contains"] = +[](table const& t, std::variant<string, i32> s) { return t.has(s); };
        tab["keys"]     = +[](table const& t) { return t.get_keys<std::variant<i32, string>>(); };
    });
}

auto script::call_buffer(string_view script, string const& name) const -> std::optional<error_code>
{
    if (_view.load_buffer(script, name)) {
        if (*Environment) {
            function<void> func {function<void>::Acquire(_view, -1)};
            if (!func.set_environment(**Environment)) {
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

void script::set_hook(HookFunc&& func, debug_mask mask)
{
    auto const guard {_view.create_scoped_stack()};

    _view.new_metatable("_tcob");
    i32 const tableIdx {_view.get_top()};

    _view.push_convert("_hook");
    _hookFunc = std::move(func);
    _view.push_convert(reinterpret_cast<void*>(&_hookFunc));
    _view.set_table(tableIdx);
    _view.set_hook(&Hook, debug::GetMask(mask), 1);
}

void script::remove_hook()
{
    _view.set_hook(nullptr, 0, 0);
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
