// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Script.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

    #include <expected>
    #include <memory>

    #include "tcob/core/io/FileStream.hpp"
    #include "tcob/core/io/FileSystem.hpp"
    #include "tcob/core/io/Stream.hpp"
    #include "tcob/scripting/Lua.hpp"
    #include "tcob/scripting/Scripting.hpp"
    #include "tcob/scripting/Types.hpp"
    #include "tcob/scripting/Wrapper.hpp"

namespace tcob::scripting {

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
inline auto script::run_file(path const& file) const -> std::expected<R, error_code>
{
    return run<R>(io::read_as_string(file), file);
}

template <typename R>
inline auto script::run(string_view script, string const& name) const -> std::expected<R, error_code>
{
    auto const guard {_view.create_stack_guard()};

    auto const result {call_buffer(script, name)};
    if constexpr (std::is_void_v<R>) {
        return !result ? std::expected<void, error_code> {} : std::unexpected {*result};
    } else {
        if (result) { return std::unexpected {*result}; }

        R retValue {};
        if (!_view.pull_convert_idx(guard.get_top() + 1, retValue)) {
            return std::unexpected {error_code::TypeMismatch};
        }

        return std::expected<R, error_code> {std::move(retValue)};
    }
}
template <typename T>
inline auto script::create_wrapper(string const& name)
{
    auto retValue {std::make_shared<wrapper<T>>(_view, &_globalTable, name)};
    _wrappers.push_back(retValue);
    return retValue;
}

template <typename R>
inline auto script::load_binary(path const& file) const -> function<R>
{
    io::ifstream ifs {file};
    return load_binary<R>(ifs, file);
}

template <typename R>
inline auto script::load_binary(io::istream& in, string const& name) const -> function<R>
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
