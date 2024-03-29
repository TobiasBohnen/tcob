// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>

#include "tcob/core/FlatMap.hpp"
#include "tcob/core/io/Stream.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <typename ReturnType, typename... Args>
class type_factory {
    using func = std::function<ReturnType(Args...)>;

public:
    void add(std::vector<string> const& names, func&& func);

    auto create(string const& name, Args&&... args) -> ReturnType;

    auto create_from_sig_or_ext(istream& in, string const& ext, Args&&... args) -> ReturnType;

private:
    flat_map<string, func> _functions;
};

}

#include "TypeFactory.inl"
