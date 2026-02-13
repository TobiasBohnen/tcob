// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/procgen/LSystem.hpp"

#include <algorithm>

#include "tcob/core/random/Random.hpp"

namespace tcob::gfx {

l_system::l_system()
    : _rng {}
{
}

l_system::l_system(u64 seed)
    : _rng {seed}
{
}

void l_system::add_rule(char c, l_rule const& rule)
{
    _rules[c].push_back(rule);

    // sort by priority
    for (auto& [_, rules] : _rules) {
        std::ranges::sort(rules, [](auto& a, auto& b) { return a.Priority > b.Priority; });
    }
}

auto l_system::generate(string_view axiom, i32 iterations) -> string
{
    string current {axiom};
    for (i32 i {0}; i < iterations; ++i) {
        string next;
        for (usize pos {0}; pos < current.size(); ++pos) {
            next += get_replacement(i, current[pos], current, pos);
        }
        current = next;
    }
    return current;
}

auto l_system::get_replacement(i32 iteration, char c, string_view prev, usize pos) -> string
{
    auto it {_rules.find(c)};
    if (it == _rules.end()) { return {c}; }
    auto& rules {it->second};

    f32 const rnd {_rng(0.0f, 1.0f)};
    f32       cumulative {0.0f};

    for (auto const& rule : rules) {
        if (rule.MinIteration && iteration < *rule.MinIteration) { continue; }
        if (rule.MaxIteration && iteration > *rule.MaxIteration) { continue; }

        bool const leftMatch {!rule.LeftContext
                              || (pos >= rule.LeftContext->size() && prev.substr(pos - rule.LeftContext->size(), rule.LeftContext->size()) == rule.LeftContext)};
        bool const rightMatch {!rule.RightContext
                               || (pos + rule.RightContext->size() < prev.size() && prev.substr(pos + 1, rule.RightContext->size()) == rule.RightContext)};

        if (!leftMatch || !rightMatch) { continue; }

        cumulative += rule.Probability;
        if (rnd <= cumulative) { return rule.Replacement; }
    }

    return {c};
}

}
