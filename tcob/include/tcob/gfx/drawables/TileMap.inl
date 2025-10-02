// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "TileMap.hpp"

#include <initializer_list>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Serialization.hpp"
#include "tcob/gfx/Geometry.hpp"

namespace tcob::gfx {

template <typename G>
inline tilemap<G>::tilemap(set_type set)
    : _tileSet {std::move(set)}
{
    Grid.Changed.connect([this](auto const&) { mark_dirty(); });
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

    rect_f rect {Grid->layout_tile(tile, coord)};
    rect.move_by(*Position);

    geometry::set_position(q, rect);
    geometry::set_color(q, tile.Color);
    if (idx == 0) {
        geometry::set_color(q, colors::Transparent);
    } else if (Material->Texture && Material->Texture->regions().contains(tile.TextureRegion)) {
        geometry::set_texcoords(q, Material->Texture->regions()[tile.TextureRegion], tile.FlipHorizontally, tile.FlipVertically);
    } else {
        geometry::set_texcoords(q, {.UVRect = {0, 0, 1, 1}, .Level = 1});
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
