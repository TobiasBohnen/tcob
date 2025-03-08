// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

    #include <functional>
    #include <memory>
    #include <optional>

    #include "tcob/core/Signal.hpp"
    #include "tcob/scripting/Script.hpp"
    #include "tcob/scripting/Scripting.hpp"
    #include "tcob/scripting/lua/Lua.hpp"
    #include "tcob/scripting/lua/LuaConversions.hpp"
    #include "tcob/scripting/lua/LuaTypes.hpp"
    #include "tcob/scripting/lua/LuaWrapper.hpp"

namespace tcob::scripting::lua {
////////////////////////////////////////////////////////////

class TCOB_API script : public scripting::script<script> {
    friend class scripting::script<script>;

public:
    using HookFunc    = std::function<void(debug const&)>;
    using LoaderFunc  = std::function<table(string const&)>;
    using SeacherFunc = std::function<LoaderFunc*(string const&)>;

    struct require_event {
        string               Name;
        std::optional<table> Table;
    };
    struct warning_event {
        string Message;
        bool   ToCont {false};
    };

    script();
    script(script&& other) noexcept                    = delete;
    auto operator=(script&& other) noexcept -> script& = delete;
    ~script();

    signal<require_event>       Require;
    signal<warning_event const> Warning;

    auto global_table() -> table&;
    auto get_environment() const -> std::optional<table>; // TODO: set_get_
    void set_environment(table const& env);

    auto get_view() const -> state_view;                  // TODO: get_
    auto gc() const -> garbage_collector;

    auto create_table() const -> table;

    template <typename... Args>
    void open_libraries(Args... args);

    template <typename R = void>
    auto load_binary(path const& file) const -> function<R>;
    template <typename R = void>
    auto load_binary(io::istream& in, string const& name = "") const -> function<R>;

    void set_hook(HookFunc&& func, debug_mask mask = {});
    void remove_hook();

    void raise_error(string const& message);

private:
    template <typename R = void>
    auto impl_run(string_view script, string const& name) const -> result<R>;
    template <typename T>
    auto impl_create_wrapper(string const& name) -> std::shared_ptr<wrapper<T>>;

    auto call_buffer(string_view script, string const& name) const -> error_code;
    auto load_binary_buffer(string_view script, string const& name) const -> bool;

    template <typename... Args>
    void load_library(library lib, Args... args);
    void load_library(library lib);

    void register_searcher();

    state_view           _view;
    table                _globalTable;
    std::optional<table> _environment;

    HookFunc    _hookFunc;
    SeacherFunc _searcher;
    LoaderFunc  _loader;
};

}

////////////////////////////////////////////////////////////

namespace tcob::literals {
auto operator""_lua(char const* str, usize) -> std::unique_ptr<tcob::scripting::lua::script>;
}

    #include "LuaScript.inl"

#endif
