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
inline auto tilemap<G>::get_tile_bounds(uid layerId, point_i pos) const -> rect_f
{
    return Grid->layout_tile(_tileSet.get_tile(get_tile_index(layerId, pos)), pos);
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
    if (idx == 0) {
        geometry::set_color(q, colors::Transparent);
    } else if (Material->Texture && Material->Texture->has_region(tile.TextureRegion)) {
        geometry::set_texcoords(q, Material->Texture->get_region(tile.TextureRegion), tile.FlipHorizontally, tile.FlipVertically);
    } else {
        geometry::set_texcoords(q, {{0, 0, 1, 1}, 1});
    }
}

////////////////////////////////////////////////////////////

template <typename T>
inline tileset<T>::tileset(std::unordered_map<tile_index_t, tile_type> set)
    : _set {std::move(set)}
{
}

template <typename T>
inline tileset<T>::tileset(std::initializer_list<std::pair<tile_index_t, tile_type>> items)
{
    _set.reserve(items.size());
    for (auto const& item : items) {
        _set[item.first] = item.second;
    }
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

}
