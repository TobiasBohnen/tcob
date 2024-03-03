// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

    #include <optional>

    #include "tcob/core/Signal.hpp"
    #include "tcob/scripting/Script.hpp"
    #include "tcob/scripting/lua/Lua.hpp"
    #include "tcob/scripting/lua/LuaTypes.hpp"
    #include "tcob/scripting/lua/LuaWrapper.hpp"

namespace tcob::scripting::lua {
////////////////////////////////////////////////////////////

enum class library : u8 {
    Table,
    String,
    Math,
    Coroutine,
    IO,
    OS,
    Utf8,
    Debug,
    Package
};

////////////////////////////////////////////////////////////

class TCOB_API gc final {
public:
    explicit gc(state_view l);

    auto get_count() const -> i32;
    auto is_running() const -> bool;

    void start_incremental_mode(i32 pause, i32 stepmul, i32 stepsize) const;
    void start_generational_mode(i32 minormul, i32 majormul) const;

    void collect() const;

    void stop() const;
    void restart() const;

private:
    state_view _luaState;
};

////////////////////////////////////////////////////////////

class TCOB_API script : public scripting::script<script> {
    friend class scripting::script<script>;

public:
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

    auto get_global_table() -> table&;
    auto get_view() const -> state_view;
    auto get_GC() const -> gc;

    auto create_table() const -> table;

    template <typename... Args>
    void open_libraries(Args... args);

    template <typename R = void>
    auto load_binary(path const& file) const -> function<R>;
    template <typename R = void>
    auto load_binary(istream& in, string const& name = "") const -> function<R>;

    void set_hook(std::function<void(debug const&)>&& func);
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

    state_view _view;
    table      _globalTable;

    std::function<void(debug const&)>                                  _hookFunc;
    std::function<std::function<table(string const&)>*(string const&)> _searcher;
    std::function<table(string const&)>                                _loader;
};

}

////////////////////////////////////////////////////////////

namespace tcob::literals {
auto operator""_lua(char const* str, usize) -> std::unique_ptr<tcob::scripting::lua::script>;
}

    #include "LuaScript.inl"

#endif
