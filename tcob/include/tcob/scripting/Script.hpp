// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/scripting/Conversions.hpp"
#include "tcob/scripting/Lua.hpp"
#include "tcob/scripting/Scripting.hpp"
#include "tcob/scripting/Types.hpp"
#include "tcob/scripting/Wrapper.hpp"

namespace tcob::scripting {
////////////////////////////////////////////////////////////

class TCOB_API script : public non_copyable {

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
    ~script();

    signal<require_event>       Require;
    signal<warning_event const> Warning;

    template <typename R = void>
    auto run_file(path const& file) const -> std::expected<R, error_code>;
    template <typename R = void>
    auto run(string_view script, string const& name = "main") const -> std::expected<R, error_code>;

    template <typename T>
    auto create_wrapper(string const& name, bool autoMeta = true);

    auto global_table() -> table&;
    auto get_environment() const -> std::optional<table>; // TODO: set_get_
    void set_environment(table const& env);

    auto view() const -> state_view;
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
    void clear_wrappers();

    auto call_buffer(string_view script, string const& name) const -> std::optional<error_code>;
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

    std::vector<std::shared_ptr<detail::wrapper_base>> _wrappers;
};

}

////////////////////////////////////////////////////////////

namespace tcob::literals {
auto operator""_lua(char const* str, usize) -> std::unique_ptr<tcob::scripting::script>;
}

#include "Script.inl"
