// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Material.hpp"

#include "tcob/core/assets/Asset.hpp"

namespace tcob::gfx {

auto material::Empty() -> assets::owning_asset_ptr<material>
{
    static assets::owning_asset_ptr<material> instance;
    return instance;
}

} // namespace gfx
