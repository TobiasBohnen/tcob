// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/procgen/LSystem.hpp"

#include <utility>

namespace tcob::gfx {

l_system::l_system(string axiom, std::unordered_map<char, string> const& rules)
    : _axiom {std::move(axiom)}
    , _rules {rules}
{
}

auto l_system::generate(i32 iterations) -> string
{
    string current {_axiom};
    for (i32 i {0}; i < iterations; ++i) {
        string next;
        for (char c : current) {
            if (_rules.find(c) != _rules.end()) {
                next += _rules.at(c);
            } else {
                next.push_back(c);
            }
        }
        current = next;
    }
    return current;
}

} // namespace gfx
