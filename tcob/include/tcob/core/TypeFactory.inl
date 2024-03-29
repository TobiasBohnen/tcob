// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "TypeFactory.hpp"

#include <algorithm>

#include "tcob/core/io/Magic.hpp"

namespace tcob {

template <typename ReturnType, typename... Args>
inline void type_factory<ReturnType, Args...>::add(std::vector<string> const& names, func&& func)
{
    for (auto const& name : names) {
        _functions[name] = std::move(func);
    }
}

template <typename ReturnType, typename... Args>
inline auto type_factory<ReturnType, Args...>::create(string const& name, Args&&... args) -> ReturnType
{
    auto const it {_functions.find(name)};
    if (it != _functions.end()) {
        return it->second(args...);
    }

    return nullptr;
}

template <typename ReturnType, typename... Args>
inline auto type_factory<ReturnType, Args...>::create_from_sig_or_ext(istream& in, string const& ext, Args&&... args) -> ReturnType
{
    auto retValue {create(io::magic::get_extension(in))};
    if (!retValue) { retValue = create(ext, std::move(args)...); }
    return retValue;
}

}
