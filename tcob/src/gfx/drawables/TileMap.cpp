// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/TileMap.hpp"

#include <cassert>
#include <utility>

#include "tcob/core/random/Random.hpp"

namespace tcob::gfx {
namespace detail {

    tilemap_base::tilemap_base(tileset set, buffer_usage_hint usage)
        : _tileSet {std::move(set)}
        , _renderer {usage}
    {
        Material.Changed.connect([&](auto const& value) { _renderer.set_material(value.get_ptr()); });
        Position.Changed.connect([&](auto const&) { _isDirty = true; });
    }

    void tilemap_base::change_tileset(tile_index_t idx, tile const& t)
    {
        _tileSet.Set[idx] = t;
        mark_dirty();
    }

    auto tilemap_base::add_layer(tilemap_layer const& layer) -> id_t
    {
        assert(std::ssize(layer.Tiles) == layer.Size.Width * layer.Size.Height);
        id_t const id {GetRandomID()};

        _layers.push_back({
            .ID           = id,
            .Size         = layer.Size,
            .Offset       = layer.Offset,
            .TileMapStart = static_cast<i32>(_tileMap.size()),
            .Visible      = true,
        });

        _tileMap.insert(_tileMap.end(), layer.Tiles.begin(), layer.Tiles.end());

        mark_dirty();
        return id;
    }

    void tilemap_base::set_tile(id_t id, point_i pos, tile_index_t setIdx)
    {
        auto const& layer {*get_layer(id)};
        i32 const   idx {layer.TileMapStart + pos.X + (pos.Y * layer.Size.Width)};
        _tileMap[idx] = setIdx;

        mark_dirty();
    }

    auto tilemap_base::get_tile(id_t id, point_i pos) const -> tile_index_t
    {
        auto const& layer {*get_layer(id)};
        return _tileMap[layer.TileMapStart + pos.X + (pos.Y * layer.Size.Width)];
    }

    auto tilemap_base::is_layer_visible(id_t id) const -> bool
    {
        return get_layer(id)->Visible;
    }

    void tilemap_base::set_layer_visible(id_t id, bool visible)
    {
        get_layer(id)->Visible = visible;
        mark_dirty();
    }

    auto tilemap_base::get_layer_size(id_t id) const -> size_i
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

        _quads.clear();
        _quads.reserve(_tileMap.size());
        for (auto const& layer : _layers) {
            if (!layer.Visible) { continue; }

            for (i32 i {0}; i < (layer.Size.Width * layer.Size.Height); ++i) {
                auto const setIdx {_tileMap[i + layer.TileMapStart]};
                if (setIdx == 0) { continue; }

                setup_quad(_quads.emplace_back(),
                           {(i % layer.Size.Width) + layer.Offset.X, (i / layer.Size.Width) + layer.Offset.Y},
                           _tileSet.Set[setIdx]);
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
            _renderer.set_geometry(_quads);
            _updateGeometry = false;
        }

        _renderer.render_to_target(target);
    }

    void tilemap_base::mark_dirty()
    {
        _isDirty = true;
    }

    auto tilemap_base::get_layer(id_t id) -> layer*
    {
        for (auto& layer : _layers) {
            if (layer.ID == id) { return &layer; }
        }

        return nullptr;
    }

    auto tilemap_base::get_layer(id_t id) const -> layer const*
    {
        for (auto const& layer : _layers) {
            if (layer.ID == id) { return &layer; }
        }

        return nullptr;
    }
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

auto orthogonal_grid::get_quad_bounds(point_f offset, point_i coord) const -> rect_f
{
    return {{offset.X + (TileSize.Width * coord.X),
             offset.Y + (TileSize.Height * coord.Y)},
            TileSize};
}

////////////////////////////////////////////////////////////

auto isometric_grid::get_quad_bounds(point_f offset, point_i coord) const -> rect_f
{
    return {{offset.X + ((TileSize.Width * SurfaceCenter.X) * (coord.X - coord.Y)),
             offset.Y + ((TileSize.Height * SurfaceCenter.Y) * (coord.Y + coord.X))},
            TileSize};
}

////////////////////////////////////////////////////////////

auto hexagonal_grid::get_quad_bounds(point_f offset, point_i coord) const -> rect_f
{
    return FlatTop
        ? rect_f {{offset.X + TileSize.Width * (3.0f / 4.0f * coord.X),
                   offset.Y + TileSize.Height * (coord.Y + 0.5f * (coord.X & 1))},
                  TileSize}
        : rect_f {{offset.X + TileSize.Width * (coord.X + 0.5f * (coord.Y & 1)),
                   offset.Y + TileSize.Height * (3.0f / 4.0f * coord.Y)},
                  TileSize};
}
}
