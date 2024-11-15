// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

    #include "tcob/scripting/Script.hpp"
    #include "tcob/scripting/squirrel/Squirrel.hpp"
    #include "tcob/scripting/squirrel/SquirrelConversions.hpp"
    #include "tcob/scripting/squirrel/SquirrelTypes.hpp"
    #include "tcob/scripting/squirrel/SquirrelWrapper.hpp"

namespace tcob::scripting::squirrel {
////////////////////////////////////////////////////////////

enum class library : u8 {
    IO,
    Blob,
    Math,
    System,
    String
};

////////////////////////////////////////////////////////////

class TCOB_API script : public scripting::script<script> {
    friend class scripting::script<script>;

public:
    using HookFunc = std::function<void(debug_event /*type*/, string const& /*sourcename*/, SQInteger /*line*/, string const& /*funcname*/)>;

    script();
    script(script&& other) noexcept                    = delete;
    auto operator=(script&& other) noexcept -> script& = delete;
    ~script();

    auto get_root_table() -> table&;  // TODO: get_
    auto get_view() const -> vm_view; // TODO: get_

    auto create_array() const -> array;
    auto create_table() const -> table;
    auto create_class() const -> clazz;

    template <typename... Args>
    void open_libraries(Args... args);
    void enable_debug_info() const;

    void set_hook(HookFunc&& func);
    void remove_hook();

private:
    template <typename R = void>
    auto impl_run(string_view script, string const& name) const -> result<R>;
    template <typename T>
    auto impl_create_wrapper(string const& name) -> std::shared_ptr<wrapper<T>>;

    auto call_buffer(string_view script, string const& name, bool retValue) const -> error_code;

    template <typename... Args>
    void load_library(library lib, Args... args);
    void load_library(library lib);

    vm_view _view;
    table   _rootTable;

    HookFunc _hookFunc;
};

}

////////////////////////////////////////////////////////////

namespace tcob::literals {
auto operator""_squirrel(char const* str, usize) -> std::unique_ptr<tcob::scripting::squirrel::script>;
}

////////////////////////////////////////////////////////////

    #include "SquirrelScript.inl"

#endif
