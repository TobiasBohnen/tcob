// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "TypeFactory.hpp"

#include <algorithm>
#include <vector>

#include "tcob/core/io/Magic.hpp"

namespace tcob {

template <typename ReturnType, typename... Args>
inline void type_factory<ReturnType, Args...>::add(string const& ext, func&& func)
{
    _functions[ext] = std::move(func);
}

template <typename ReturnType, typename... Args>
inline void type_factory<ReturnType, Args...>::add(std::vector<string> const& exts, func const& func)
{
    for (auto const& name : exts) {
        _functions[name] = func;
    }
}

template <typename ReturnType, typename... Args>
inline auto type_factory<ReturnType, Args...>::create(string const& ext, Args&&... args) -> ReturnType
{
    auto const it {_functions.find(ext)};
    if (it != _functions.end()) {
        return it->second(std::forward<Args>(args)...);
    }

    return nullptr;
}

template <typename ReturnType, typename... Args>
inline auto type_factory<ReturnType, Args...>::create(io::istream& in, string const& fallbackExt, Args&&... args) -> ReturnType
{
    auto retValue {create(io::magic::get_extension(in))};
    if (!retValue) { retValue = create(fallbackExt, std::move(args)...); }
    return retValue;
}

}
