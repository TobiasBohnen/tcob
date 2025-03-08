// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <unordered_map>
#include <vector>

#include "tcob/core/random/Random.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

struct l_rule {
    string                Replacement;
    f32                   Probability {1.0f};
    std::optional<string> LeftContext {std::nullopt};
    std::optional<string> RightContext {std::nullopt};
};

class TCOB_API l_system {
public:
    l_system();
    l_system(u64 seed);

    void add_rule(char c, l_rule const& rule);
    auto generate(string axiom, i32 iterations) -> string;

private:
    auto get_replacement(char c, string const& prev, usize pos) -> string;

    rng                                           _rng;
    std::unordered_map<char, std::vector<l_rule>> _rules;
};

}
