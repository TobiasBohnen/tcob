// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <unordered_map>

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API l_system {
public:
    l_system(string axiom, std::unordered_map<char, string> const& rules);

    auto generate(i32 iterations) -> string;

private:
    string                           _axiom;
    std::unordered_map<char, string> _rules;
};

}
