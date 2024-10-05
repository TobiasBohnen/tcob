// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Script.hpp"

#include "tcob/core/io/FileSystem.hpp"

namespace tcob::scripting {

template <typename ScriptImpl>
inline auto script<ScriptImpl>::get_impl() -> ScriptImpl*
{
    return static_cast<ScriptImpl*>(this);
}

template <typename ScriptImpl>
inline auto script<ScriptImpl>::get_impl() const -> ScriptImpl const*
{
    return static_cast<ScriptImpl const*>(this);
}

template <typename ScriptImpl>
template <typename R>
inline auto script<ScriptImpl>::run_file(path const& file) const -> result<R>
{
    return run<R>(io::read_as_string(file), file);
}

template <typename ScriptImpl>
template <typename R>
inline auto script<ScriptImpl>::run(string_view script, string const& name) const -> result<R>
{
    return get_impl()->template impl_run<R>(script, name);
}

template <typename ScriptImpl>
template <typename T>
inline auto script<ScriptImpl>::create_wrapper(string const& name)
{
    auto retValue {get_impl()->template impl_create_wrapper<T>(name)};
    _wrappers.push_back(retValue);
    return retValue;
}

template <typename ScriptImpl>
inline void script<ScriptImpl>::clear_wrappers()
{
    _wrappers.clear();
}

}
