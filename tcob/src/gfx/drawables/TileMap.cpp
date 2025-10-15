// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/TileMap.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderTarget.hpp"

namespace tcob::gfx {

static auto IndexToPosition(i32 i, render_direction direction, size_i layerSize) -> point_i
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
    Material.Changed.connect([this](auto const&) { mark_dirty(); });
    Position.Changed.connect([this](auto const&) { mark_dirty(); });
}

auto tilemap_base::create_layer() -> tilemap_layer&
{
    mark_dirty();
    return *_layers.emplace_back(std::unique_ptr<tilemap_layer>(new tilemap_layer {this}));
}

void tilemap_base::remove_layer(tilemap_layer const& layer)
{
    helper::erase_first(_layers, [&layer](auto const& val) {
        return static_cast<bool>(val.get() == &layer);
    });

    mark_dirty();
}

void tilemap_base::clear()
{
    _layers.clear();
    mark_dirty();
}

void tilemap_base::bring_to_front(tilemap_layer const& layer)
{
    auto it {std::ranges::find_if(_layers, [&layer](auto const& val) { return val.get() == &layer; })};
    if (it != _layers.end()) {
        std::rotate(it, it + 1, _layers.end());
    }
    mark_dirty();
}

void tilemap_base::send_to_back(tilemap_layer const& layer)
{
    auto it {std::ranges::find_if(_layers, [&layer](auto const& val) { return val.get() == &layer; })};
    if (it != _layers.end()) {
        std::rotate(_layers.begin(), it, it + 1);
    }
    mark_dirty();
}

void tilemap_base::notify_layer_changed(tilemap_layer* /* layer */)
{
    mark_dirty();
}

void tilemap_base::on_update(milliseconds /* deltaTime */)
{
    if (_layers.empty() || !Material) { return; }
    if (!_isDirty) { return; }

    _quads.clear();
    _isDirty = false;

    for (auto const& layer : _layers) {
        if (!layer->Visible) { continue; }

        auto const& tiles {*layer->Tiles};
        for (i32 i {0}; i < tiles.width() * tiles.height(); ++i) {
            auto const tilePos {IndexToPosition(i, layer->RenderDirection, tiles.size())};
            setup_quad(_quads.emplace_back(), tilePos + *layer->Offset, tiles[tilePos]);
        }
    }
}

auto tilemap_base::can_draw() const -> bool
{
    return !_layers.empty() && !(*Material).is_expired();
}

void tilemap_base::on_draw_to(render_target& target)
{
    for (isize i {0}; i < Material->pass_count(); ++i) {
        auto const& pass {Material->get_pass(i)};

        _renderer.set_geometry(_quads, &pass);
        _renderer.render_to_target(target);
    }
}

void tilemap_base::mark_dirty()
{
    _isDirty = true;
}

////////////////////////////////////////////////////////////

tilemap_layer::tilemap_layer(tilemap_base* parent)
{
    Tiles.Changed.connect([this, parent](auto const&) { parent->notify_layer_changed(this); });
    Offset.Changed.connect([this, parent](auto const&) { parent->notify_layer_changed(this); });
    Visible.Changed.connect([this, parent](auto const&) { parent->notify_layer_changed(this); });
    RenderDirection.Changed.connect([this, parent](auto const&) { parent->notify_layer_changed(this); });
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

auto orthogonal_grid::layout_tile(orthogonal_tile const& tile, point_i coord) const -> rect_f
{
    auto const [x, y] {point_f {coord}};
    return {{TileSize.Width * x, TileSize.Height * y},
            TileSize * tile.Scale};
}

////////////////////////////////////////////////////////////

auto isometric_grid::layout_tile(isometric_tile const& tile, point_i coord) const -> rect_f
{
    auto const [x, y] {point_f {coord}};
    return Staggered
        ? rect_f {{TileSize.Width * (x + tile.Center.X * (coord.Y & 1)),
                   TileSize.Height * (tile.Center.Y * y)},
                  TileSize}
        : rect_f {{((TileSize.Width * tile.Center.X) * (x - y)),
                   ((TileSize.Height * tile.Center.Y) * (y + x)) - (TileSize.Height * tile.Height)},
                  TileSize};
}

////////////////////////////////////////////////////////////

auto hexagonal_grid::layout_tile(hexagonal_tile const& /* tile */, point_i coord) const -> rect_f
{
    auto const [x, y] {point_f {coord}};
    return Top == hexagonal_top::Flat
        ? rect_f {{TileSize.Width * (3.0f / 4.0f * x),
                   TileSize.Height * (y + 0.5f * (coord.X & 1))},
                  TileSize}
        : rect_f {{TileSize.Width * (x + 0.5f * (coord.Y & 1)),
                   TileSize.Height * (3.0f / 4.0f * y)},
                  TileSize};
}

////////////////////////////////////////////////////////////

}
