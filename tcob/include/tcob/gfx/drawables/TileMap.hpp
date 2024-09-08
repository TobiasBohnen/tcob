// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/FlatMap.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////
using tile_index_t = u16;

struct tile {
    string TextureRegion {};
    bool   FlipHorizontally {false};
    bool   FlipVertically {false};
};

struct tileset {
    flat_map<tile_index_t, tile> Set;
};

struct tilemap_layer {
    std::vector<tile_index_t> Tiles;
    size_i                    Size;
};

////////////////////////////////////////////////////////////

class TCOB_API orthogonal_grid {
public:
    size_f TileSize {size_f::Zero};

    auto get_quad_bounds(point_f offset, point_i coord) const -> rect_f;

    auto operator==(orthogonal_grid const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API isometric_grid {
public:
    size_f  TileSize {size_f::Zero};
    point_f SurfaceCenter {0.5f, 0.5f};

    auto get_quad_bounds(point_f offset, point_i coord) const -> rect_f;

    auto operator==(isometric_grid const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

namespace detail {
    class TCOB_API tilemap_base : public drawable {
    public:
        tilemap_base(tileset set, buffer_usage_hint usage);
        ~tilemap_base() override = default;

        prop<assets::asset_ptr<material>> Material;
        prop<point_f>                     Position;

        auto add_layer(tilemap_layer const& layer, point_i tileOffset = point_i::Zero) -> usize;

        void change_tile(usize layerIdx, point_i pos, tile_index_t setIdx);
        auto get_tile(usize layerIdx, point_i pos) const -> tile_index_t;
        auto is_visible(usize layerIdx) const -> bool;
        void set_visible(usize layerIdx, bool visible);
        auto get_size(usize layerIdx) const -> size_i;

        void clear();

        void change_tileset(tile_index_t idx, tile const& t);

    protected:
        void on_update(milliseconds deltaTime) override;

        auto can_draw() const -> bool override;
        void on_draw_to(render_target& target) override;

        void mark_dirty();

    private:
        void virtual setup_quad(quad& q, point_i coord, tile const& tile) const = 0;

        std::vector<tile_index_t> _tileMap {};

        struct layer {
            size_i  Size {size_i::Zero};
            i32     TileCount {0};
            i32     TileMapStart {0};
            point_i Offset {point_i::Zero};
            bool    Visible {true};
        };

        std::vector<layer> _layers {};
        tileset            _tileSet {};

        quad_renderer     _renderer;
        std::vector<quad> _quads {};
        bool              _isDirty {true};
        bool              _updateGeometry {true};
    };
}

////////////////////////////////////////////////////////////

template <typename GridType>
class tilemap : public detail::tilemap_base {
    using grid_type = GridType;

public:
    explicit tilemap(tileset set);

    prop<grid_type> Grid;

private:
    void setup_quad(quad& q, point_i coord, tile const& tile) const override;
};

////////////////////////////////////////////////////////////

using orthogonal_tilemap = tilemap<orthogonal_grid>;
using isometric_tilemap  = tilemap<isometric_grid>;

}

#include "TileMap.inl"
