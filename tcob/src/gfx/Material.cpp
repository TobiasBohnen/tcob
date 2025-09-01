// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Material.hpp"

#include "tcob/core/assets/Asset.hpp"

namespace tcob::gfx {

auto material::Empty() -> asset_owner_ptr<material>
{
    static asset_owner_ptr<material> instance;
    return instance;
}

} // namespace gfx
