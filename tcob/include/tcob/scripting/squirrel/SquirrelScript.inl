// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SquirrelScript.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

    #include <expected>
    #include <memory>

    #include "tcob/scripting/Scripting.hpp"
    #include "tcob/scripting/squirrel/SquirrelWrapper.hpp"

namespace tcob::scripting::squirrel {

template <typename... Args>
inline void script::open_libraries(Args... args)
{
    if constexpr (sizeof...(args) == 0) {
        load_library(library::IO, library::Blob, library::Math, library::String);
    } else {
        load_library(args...);
    }
}

template <typename R>
inline auto script::impl_run(string_view script, string const& name) const -> std::expected<R, error_code>
{
    auto const guard {_view.create_stack_guard()};

    auto const result {call_buffer(script, name, !std::is_void_v<R>)};
    if constexpr (std::is_void_v<R>) {
        return !result ? std::expected<void, error_code> {} : std::unexpected {*result};
    } else {
        if (result) { return std::unexpected {*result}; }

        R retValue {};
        if (!_view.pull_convert_idx(_view.get_top(), retValue)) {
            return std::unexpected {error_code::TypeMismatch};
        }

        return std::expected<R, error_code> {std::move(retValue)};
    }
}

template <typename T>
inline auto script::impl_create_wrapper(string const& name) -> std::shared_ptr<wrapper<T>>
{
    return std::make_shared<wrapper<T>>(_view, &_rootTable, name);
}

template <typename... Args>
inline void script::load_library(library lib, Args... args)
{
    load_library(lib);

    if constexpr (sizeof...(args) > 0) {
        load_library(args...);
    }
}

}

#endif
