// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/TileMap.hpp"

#include <cassert>
#include <utility>

namespace tcob::gfx {
namespace detail {

    tilemap_base::tilemap_base(tileset set, buffer_usage_hint usage)
        : _tileSet {std::move(set)}
        , _renderer {usage}
    {
        Material.Changed.connect([&](auto const& value) { _renderer.set_material(value); });
        Position.Changed.connect([&](auto const&) { _isDirty = true; });
    }

    void tilemap_base::change_tileset(tile_index_t idx, tile const& t)
    {
        _tileSet.Set[idx] = t;
        _isDirty          = true;
    }

    auto tilemap_base::add_layer(tilemap_layer const& layer, point_i tileOffset) -> usize
    {
        assert(std::ssize(layer.Tiles) == layer.Size.Width * layer.Size.Height);

        _layers.push_back({.Size         = layer.Size,
                           .TileCount    = layer.Size.Width * layer.Size.Height,
                           .TileMapStart = static_cast<i32>(_tileMap.size()),
                           .Offset       = tileOffset});
        _tileMap.insert(_tileMap.end(), layer.Tiles.begin(), layer.Tiles.end());

        _isDirty = true;
        return _layers.size() - 1;
    }

    void tilemap_base::change_tile(usize layerIdx, point_i pos, tile_index_t setIdx)
    {
        auto& layer {_layers.at(layerIdx)};
        _tileMap[layer.TileMapStart + pos.X + (pos.Y * layer.Size.Width)] = setIdx;
        // TODO: only mark affected cells as dirty
        _isDirty                                                          = true;
    }

    auto tilemap_base::get_tile(usize layerIdx, point_i pos) const -> tile_index_t
    {
        auto const& layer {_layers.at(layerIdx)};
        return _tileMap[layer.TileMapStart + pos.X + (pos.Y * layer.Size.Width)];
    }

    auto tilemap_base::is_visible(usize layerIdx) const -> bool
    {
        return _layers.at(layerIdx).Visible;
    }

    void tilemap_base::set_visible(usize layerIdx, bool visible)
    {
        _layers.at(layerIdx).Visible = visible;
        _isDirty                     = true;
    }

    auto tilemap_base::get_size(usize layerIdx) const -> size_i
    {
        return _layers.at(layerIdx).Size;
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

        if (_isDirty) {
            _quads.clear();
            _quads.reserve(_tileMap.size());
            for (auto const& layer : _layers) {
                if (!layer.Visible) { continue; }

                for (i32 i {0}; i < layer.TileCount; ++i) {
                    auto const setIdx {_tileMap[i + layer.TileMapStart]};
                    if (setIdx == 0) { continue; }

                    setup_quad(_quads.emplace_back(),
                               {(i % layer.Size.Width) + layer.Offset.X, (i / layer.Size.Width) + layer.Offset.Y},
                               _tileSet.Set[setIdx]);
                }
            }

            _isDirty        = false;
            _updateGeometry = true;
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
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

auto orthogonal_grid::get_quad_bounds(point_f offset, point_i coord) const -> rect_f
{
    return {{offset.X + (TileSize.Width * coord.X), offset.Y + (TileSize.Height * coord.Y)}, TileSize};
}

////////////////////////////////////////////////////////////

auto isometric_grid::get_quad_bounds(point_f offset, point_i coord) const -> rect_f
{
    return {{offset.X + ((TileSize.Width * SurfaceCenter.X) * (coord.X - coord.Y)),
             offset.Y + ((TileSize.Height * SurfaceCenter.Y) * (coord.Y + coord.X))},
            TileSize};
}

////////////////////////////////////////////////////////////

}
