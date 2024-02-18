// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "LuaScript.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

    #include "tcob/core/io/FileStream.hpp"

namespace tcob::scripting::lua {

template <typename... Args>
inline void script::open_libraries(Args... args)
{
    if constexpr (sizeof...(args) == 0) {
        load_library(library::Table, library::String,
                     library::Math, library::Coroutine,
                     library::IO, library::Utf8, library::Package);
    } else {
        load_library(args...);
    }
}

template <typename R>
inline auto script::impl_run(string_view script, string const& name) const -> result<R>
{
    auto const guard {_view.create_stack_guard()};

    auto result {call_buffer(script, name)};
    if constexpr (std::is_void_v<R>) {
        return make_result(result);
    } else {
        R retValue {};
        if (result == error_code::Ok) {
            if (!_view.pull_convert_idx(guard.get_top() + 1, retValue)) {
                result = error_code::TypeMismatch;
            }
        }

        return make_result(std::move(retValue), result);
    }
}

template <typename T>
inline auto script::impl_create_wrapper(string const& name) -> std::shared_ptr<wrapper<T>>
{
    return std::make_shared<wrapper<T>>(_view, &_globalTable, name);
}

template <typename R>
inline auto script::load_binary(path const& file) const -> function<R>
{
    io::ifstream ifs {file};
    return load_binary<R>(ifs, file);
}

template <typename R>
inline auto script::load_binary(istream& in, string const& name) const -> function<R>
{
    auto const guard {_view.create_stack_guard()};

    function<R> retVal {};
    auto        script {in.read_string(in.size_in_bytes())};
    if (load_binary_buffer(script, name)) {
        _view.pull_convert_idx(-1, retVal);
    }

    return retVal;
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
