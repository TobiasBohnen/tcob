// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Material.hpp"

namespace tcob::gfx {

auto material::Empty() -> assets::manual_asset_ptr<material>
{
    static assets::manual_asset_ptr<material> instance;
    return instance;
}

} // namespace gfx
