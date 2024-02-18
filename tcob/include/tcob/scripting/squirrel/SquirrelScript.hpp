// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

    #include "tcob/scripting/Script.hpp"
    #include "tcob/scripting/squirrel/Squirrel.hpp"
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
    script();
    script(script&& other) noexcept                    = delete;
    auto operator=(script&& other) noexcept -> script& = delete;
    ~script();

    auto get_root_table() -> table&;
    auto get_view() const -> vm_view;

    template <typename... Args>
    void open_libraries(Args... args);

private:
    template <typename R = void>
    auto impl_run(string_view script, string const& name) const -> result<R>;
    template <typename T>
    auto impl_create_wrapper(string const& name) -> std::shared_ptr<wrapper<T>>;

    auto call_buffer(string_view script, string const& name, bool retValue) const -> error_code;

    template <typename... Args>
    void load_library(library lib, Args... args);
    void load_library(library lib);

    vm_view _vm;
    table   _rootTable;
};

}

////////////////////////////////////////////////////////////

namespace tcob::literals {
auto operator""_squirrel(char const* str, usize) -> std::unique_ptr<tcob::scripting::squirrel::script>;
}

////////////////////////////////////////////////////////////

    #include "SquirrelScript.inl"

#endif
