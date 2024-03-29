// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "TileMap.hpp"

namespace tcob::gfx::detail {

template <typename GridType>
inline tilemap<GridType>::tilemap(tileset set)
    : detail::tilemap_base {std::move(set), buffer_usage_hint::DynamicDraw}
{
    Grid.Changed.connect([&] { mark_dirty(); });
}

template <typename GridType>
inline void tilemap<GridType>::setup_quad(quad& q, i32 column, i32 row, tile const& tile) const
{
    geometry::set_position(q, Grid->get_quad_bounds(Position(), column, row));
    geometry::set_color(q, colors::White);
    geometry::set_texcoords(q, Material->Texture->get_region(tile.TextureRegion), tile.FlipHorizontally, tile.FlipVertically);
}

}
