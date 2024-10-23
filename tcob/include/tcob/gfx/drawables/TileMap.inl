// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <utility>

#include "TileMap.hpp"

namespace tcob::gfx {

template <typename G>
inline tilemap<G>::tilemap(set_type set)
    : _tileSet {std::move(set)}
{
    Grid.Changed.connect([&](auto const&) { mark_dirty(); });
}

template <typename G>
inline auto tilemap<G>::get_tile_index(string const& texture) const -> tile_index_t
{
    return _tileSet.get_index(texture);
}

template <typename G>
inline void tilemap<G>::change_tileset(tile_index_t idx, tile_type const& t)
{
    _tileSet.set_tile(idx, t);
    mark_dirty();
}

template <typename G>
inline void tilemap<G>::setup_quad(quad& q, point_i coord, tile_index_t idx) const
{
    auto const& tile {idx != 0 ? _tileSet.get_tile(idx) : tile_type {}};

    auto rect {Grid->layout_tile(tile, coord)};
    rect.move_by(Position());

    geometry::set_position(q, rect);
    geometry::set_color(q, tile.Color);
    if (idx != 0) {
        geometry::set_texcoords(q, Material->Texture->get_region(tile.TextureRegion), tile.FlipHorizontally, tile.FlipVertically);
    } else {
        geometry::set_color(q, colors::Transparent);
    }
}

////////////////////////////////////////////////////////////

template <typename T>
tileset<T>::tileset(std::unordered_map<tile_index_t, tile_type> const& set)
    : _set {set}
{
}

template <typename T>
auto tileset<T>::get_tile(tile_index_t idx) const -> tile_type const&
{
    return _set.at(idx);
}

template <typename T>
void tileset<T>::set_tile(tile_index_t idx, tile_type const& tile)
{
    _set[idx] = tile;
}

template <typename T>
auto tileset<T>::get_index(string const& texture) const -> tile_index_t
{
    for (auto const& [k, v] : _set) {
        if (v.TextureRegion == texture) { return k; }
    }

    return 0;
}

}
