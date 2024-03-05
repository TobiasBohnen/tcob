// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SquirrelScript.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

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
inline auto script::impl_run(string_view script, string const& name) const -> result<R>
{
    auto const guard {_view.create_stack_guard()};

    auto result {call_buffer(script, name, !std::is_void_v<R>)};
    if constexpr (std::is_void_v<R>) {
        return make_result(result);
    } else {
        R retValue {};
        if (result == error_code::Ok) {
            if (!_view.pull_convert_idx(_view.get_top(), retValue)) {
                result = error_code::TypeMismatch;
            }
        }

        return make_result(std::move(retValue), result);
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
