// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "tcob/core/Serialization.hpp"
#include "tcob/core/random/Random.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API l_rule {
public:
    string                Replacement;
    f32                   Probability {1.0f};
    std::optional<string> LeftContext {std::nullopt};
    std::optional<string> RightContext {std::nullopt};
    i32                   Priority {0};
    std::optional<i32>    MinIteration {std::nullopt};
    std::optional<i32>    MaxIteration {std::nullopt};

    auto operator==(l_rule const& other) const -> bool = default;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&l_rule::Replacement> {"replacement"},
            member<&l_rule::Probability, 1.0f> {"probability"},
            member<&l_rule::LeftContext, std::nullopt> {"left_context"},
            member<&l_rule::RightContext, std::nullopt> {"right_context"},
            member<&l_rule::Priority, 0> {"priority"},
            member<&l_rule::MinIteration, std::nullopt> {"min_iteration"},
            member<&l_rule::MaxIteration, std::nullopt> {"max_iteration"}};
    }
};

class TCOB_API l_system {
public:
    l_system();
    explicit l_system(u64 seed);

    void add_rule(char c, l_rule const& rule);
    auto generate(string_view axiom, i32 iterations) -> string;

private:
    auto get_replacement(i32 iteration, char c, string const& prev, usize pos) -> string;

    rng                                           _rng;
    std::unordered_map<char, std::vector<l_rule>> _rules;
};

}
