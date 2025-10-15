// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "TileMap.hpp"

#include <tuple>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Serialization.hpp"
#include "tcob/gfx/Geometry.hpp"

namespace tcob::gfx {

template <typename G>
inline tilemap<G>::tilemap()
{
    Grid.Changed.connect([this](auto const&) { mark_dirty(); });
    Tileset.Changed.connect([this](auto const&) { mark_dirty(); });
}

template <typename G>
inline void tilemap<G>::setup_quad(quad& q, point_i coord, tile_index_t idx) const
{
    if (!*Material) { return; }
    auto const& pass {Material->first_pass()}; // TODO texRegion pass

    auto const& tile {idx != 0 ? Tileset->at(idx) : tile_type {}};

    rect_f rect {Grid->layout_tile(tile, coord)};
    rect.move_by(*Position);

    geometry::set_position(q, rect);
    geometry::set_color(q, tile.Color);
    if (idx == 0) {
        geometry::set_color(q, colors::Transparent);
    } else {
        geometry::set_texcoords(q, pass, tile.TextureRegion);
    }
}

////////////////////////////////////////////////////////////

auto constexpr orthogonal_tile::Members()
{
    return std::tuple {
        member<&orthogonal_tile::TextureRegion> {"texture"},
        member<&orthogonal_tile::FlipHorizontally> {"h_flip"},
        member<&orthogonal_tile::FlipVertically> {"v_flip"},
        member<&orthogonal_tile::Color> {"color"},
        member<&orthogonal_tile::Scale> {"scale"}};
}

auto constexpr isometric_tile::Members()
{
    return std::tuple {
        member<&isometric_tile::TextureRegion> {"texture"},
        member<&isometric_tile::FlipHorizontally> {"h_flip"},
        member<&isometric_tile::FlipVertically> {"v_flip"},
        member<&isometric_tile::Color> {"color"},
        member<&isometric_tile::Center> {"center"},
        member<&isometric_tile::Height> {"height"}};
}

auto constexpr hexagonal_tile::Members()
{
    return std::tuple {
        member<&hexagonal_tile::TextureRegion> {"texture"},
        member<&hexagonal_tile::FlipHorizontally> {"h_flip"},
        member<&hexagonal_tile::FlipVertically> {"v_flip"},
        member<&hexagonal_tile::Color> {"color"}};
}

}
