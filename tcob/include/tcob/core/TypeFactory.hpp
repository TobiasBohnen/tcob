// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <unordered_map>
#include <vector>

#include "tcob/core/Interfaces.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <typename ReturnType, typename... Args>
class type_factory : public non_copyable {
    using func = std::function<ReturnType(Args...)>;

public:
    void add(string const& ext, func&& func);
    void add(std::vector<string> const& exts, func const& func);

    auto create(string const& ext, Args&&... args) -> ReturnType;
    auto create(io::istream& in, string const& fallbackExt, Args&&... args) -> ReturnType;

private:
    std::unordered_map<string, func> _functions;
};

}

#include "TypeFactory.inl"
