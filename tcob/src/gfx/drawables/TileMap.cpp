// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/TileMap.hpp"

#include <cassert>

#include "tcob/core/random/Random.hpp"

namespace tcob::gfx {

auto static IndexToPosition(i32 i, render_direction direction, size_i layerSize) -> point_i
{
    i32 row {}, col {};

    switch (direction) {
    case render_direction::RightDown:
        row = i / layerSize.Width;
        col = i % layerSize.Width;
        break;
    case render_direction::RightUp:
        row = i / layerSize.Width;
        col = (layerSize.Width - 1) - (i % layerSize.Width);
        break;
    case render_direction::LeftDown:
        row = (layerSize.Height - 1) - (i / layerSize.Width);
        col = i % layerSize.Width;
        break;
    case render_direction::LeftUp:
        row = (layerSize.Height - 1) - (i / layerSize.Width);
        col = (layerSize.Width - 1) - (i % layerSize.Width);
        break;
    }

    return {col, row};
}

////////////////////////////////////////////////////////////

tilemap_base::tilemap_base()
    : _renderer {buffer_usage_hint::DynamicDraw}
{
    Material.Changed.connect([&](auto const& value) { _renderer.set_material(value.get_ptr()); });
    Position.Changed.connect([&](auto const&) { mark_dirty(); });
    RenderDirection.Changed.connect([&](auto const&) { mark_dirty(); });
}

auto tilemap_base::add_layer(tilemap_layer const& layer) -> uid
{
    uid const id {GetRandomID()};

    _layers.push_back({
        .ID           = id,
        .Size         = layer.Tiles.get_extent(),
        .Offset       = layer.Offset,
        .TileMapStart = static_cast<i32>(_tileMap.size()),
        .Visible      = true,
    });

    _tileMap.insert(_tileMap.end(), layer.Tiles.begin(), layer.Tiles.end());

    mark_dirty();
    return id;
}

auto tilemap_base::get_tile_index(uid layerId, point_i pos) const -> tile_index_t
{
    auto const& layer {*get_layer(layerId)};
    i32 const   idx {layer.get_index(pos)};
    if (idx >= std::ssize(_tileMap)) { return -1; }
    return _tileMap[idx];
}

void tilemap_base::set_tile_index(uid layerId, point_i pos, tile_index_t setIdx)
{
    auto const& layer {*get_layer(layerId)};
    i32 const   idx {layer.get_index(pos)};
    if (idx >= std::ssize(_tileMap)) { return; }
    if (setIdx == _tileMap[idx]) { return; }

    _tileMap[idx] = setIdx;

    i32 quadIndex {0};
    switch (RenderDirection()) {
    case render_direction::RightDown: quadIndex = pos.Y * layer.Size.Width + pos.X + layer.TileMapStart; break;
    case render_direction::LeftDown: quadIndex = pos.Y * layer.Size.Width + (layer.Size.Width - 1 - pos.X) + layer.TileMapStart; break;
    case render_direction::RightUp: quadIndex = (layer.Size.Height - 1 - pos.Y) * layer.Size.Width + pos.X + layer.TileMapStart; break;
    case render_direction::LeftUp: quadIndex = (layer.Size.Height - 1 - pos.Y) * layer.Size.Width + (layer.Size.Width - 1 - pos.X) + layer.TileMapStart; break;
    }

    auto& quad {_quads[quadIndex]};
    setup_quad(quad, {pos.X + layer.Offset.X, pos.Y + layer.Offset.Y}, setIdx);
    _updateGeometry = true;
}

auto tilemap_base::is_layer_visible(uid id) const -> bool
{
    return get_layer(id)->Visible;
}

void tilemap_base::set_layer_visible(uid id, bool visible)
{
    get_layer(id)->Visible = visible;
    mark_dirty();
}

auto tilemap_base::get_layer_size(uid id) const -> size_i
{
    return get_layer(id)->Size;
}

void tilemap_base::clear()
{
    _tileMap.clear();
    _layers.clear();
    _renderer.reset_geometry();
}

void tilemap_base::on_update(milliseconds /* deltaTime */)
{
    if (_tileMap.empty() || !Material()) { return; }
    if (!_isDirty) { return; }

    _isDirty        = false;
    _updateGeometry = true;

    for (auto const& layer : _layers) {
        if (!layer.Visible) { continue; }

        for (i32 i {0}; i < (layer.Size.Width * layer.Size.Height); ++i) {
            point_i const      tilePos {IndexToPosition(i, RenderDirection(), layer.Size)};
            tile_index_t const tileIdx {_tileMap[layer.get_index(tilePos)]};
            setup_quad(_quads.emplace_back(), {tilePos.X + layer.Offset.X, tilePos.Y + layer.Offset.Y}, tileIdx);
        }
    }
}

auto tilemap_base::can_draw() const -> bool
{
    return !_tileMap.empty() && !Material().is_expired();
}

void tilemap_base::on_draw_to(render_target& target)
{
    if (_updateGeometry) {
        _updateGeometry = false;
        _renderer.set_geometry(_quads);
    }

    _renderer.render_to_target(target);
}

void tilemap_base::mark_dirty()
{
    _isDirty = true;

    _quads.clear();
    _quads.reserve(_tileMap.size());
}

auto tilemap_base::get_layer(uid id) -> layer*
{
    for (auto& layer : _layers) {
        if (layer.ID == id) { return &layer; }
    }

    return nullptr;
}

auto tilemap_base::get_layer(uid id) const -> layer const*
{
    for (auto const& layer : _layers) {
        if (layer.ID == id) { return &layer; }
    }

    return nullptr;
}

auto tilemap_base::layer::get_index(point_i pos) const -> i32
{
    return static_cast<i32>(TileMapStart + pos.X + (pos.Y * Size.Width));
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

auto ortho_grid::layout_tile(ortho_tile const& tile, point_i coord) const -> rect_f
{
    return {{TileSize.Width * coord.X,
             TileSize.Height * coord.Y},
            TileSize * tile.Scale};
}

////////////////////////////////////////////////////////////

auto iso_grid::layout_tile(iso_tile const& tile, point_i coord) const -> rect_f
{
    return Staggered
        ? rect_f {{TileSize.Width * (coord.X + 0.5f * (coord.Y & 1)),
                   TileSize.Height * (0.5f * coord.Y)},
                  TileSize}
        : rect_f {{((TileSize.Width * tile.Center.X) * (coord.X - coord.Y)),
                   ((TileSize.Height * tile.Center.Y) * (coord.Y + coord.X)) - (TileSize.Height * tile.Height)},
                  TileSize};
}

////////////////////////////////////////////////////////////

auto hex_grid::layout_tile(hex_tile const& /* tile */, point_i coord) const -> rect_f
{
    return Top == hex_top::Flat
        ? rect_f {{TileSize.Width * (3.0f / 4.0f * coord.X),
                   TileSize.Height * (coord.Y + 0.5f * (coord.X & 1))},
                  TileSize}
        : rect_f {{TileSize.Width * (coord.X + 0.5f * (coord.Y & 1)),
                   TileSize.Height * (3.0f / 4.0f * coord.Y)},
                  TileSize};
}

////////////////////////////////////////////////////////////
}
