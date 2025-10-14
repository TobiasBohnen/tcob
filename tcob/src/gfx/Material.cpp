// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Material.hpp"

#include <iterator>

#include "tcob/core/assets/Asset.hpp"

namespace tcob::gfx {

auto material::Empty() -> asset_owner_ptr<material>
{
    static asset_owner_ptr<material> instance;
    return instance;
}

auto material::first_pass() -> pass&
{
    if (_passes.empty()) { return create_pass(); }
    return _passes[0];
}

auto material::create_pass() -> pass& { return _passes.emplace_back(); }

auto material::get_pass(isize idx) -> pass& { return _passes.at(idx); }
auto material::get_pass(isize idx) const -> pass const& { return _passes.at(idx); }

auto material::pass_count() const -> isize { return std::ssize(_passes); }
}
