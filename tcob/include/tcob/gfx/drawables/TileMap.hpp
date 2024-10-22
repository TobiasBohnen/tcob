// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <unordered_map>
#include <vector>

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
using tile_index_t = u16;

////////////////////////////////////////////////////////////

template <typename T>
class tileset {
public:
    using tile_type = T;

    tileset(std::unordered_map<tile_index_t, tile_type> const& set);

    auto get_tile(tile_index_t idx) const -> tile_type const&;
    void set_tile(tile_index_t idx, tile_type const& tile_type);
    auto get_index(string const& texture) const -> tile_index_t;

private:
    std::unordered_map<tile_index_t, tile_type> _set;
};

////////////////////////////////////////////////////////////

struct ortho_tile {
    string TextureRegion {};
    bool   FlipHorizontally {false};
    bool   FlipVertically {false};
    size_f Scale {size_f::One};
};

class TCOB_API ortho_grid {
public:
    using tile_type = ortho_tile;

    size_f TileSize {size_f::Zero};

    auto layout_tile(ortho_tile const& tile, point_i coord) const -> rect_f;

    auto operator==(ortho_grid const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

struct iso_tile {
    string  TextureRegion {};
    bool    FlipHorizontally {false};
    bool    FlipVertically {false};
    point_f Center {0.5f, 0.5f};
    f32     Height {0.0f};
};

class TCOB_API iso_grid {
public:
    using tile_type = iso_tile;

    size_f TileSize {size_f::Zero};

    auto layout_tile(iso_tile const& tile, point_i coord) const -> rect_f;

    auto operator==(iso_grid const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

struct hex_tile {
    string TextureRegion {};
    bool   FlipHorizontally {false};
    bool   FlipVertically {false};
};

enum hex_top {
    Pointy,
    Flat
};

class TCOB_API hex_grid {
public:
    using tile_type = hex_tile;

    size_f  TileSize {size_f::Zero};
    hex_top Top {hex_top::Pointy};

    auto layout_tile(hex_tile const& tile, point_i coord) const -> rect_f;

    auto operator==(hex_grid const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

struct tilemap_layer {
    std::span<tile_index_t> Tiles; // TODO: mdspan
    size_i                  Size {size_i::Zero};
    point_i                 Offset {point_i::Zero};
};

////////////////////////////////////////////////////////////

class TCOB_API tilemap_base : public drawable {
public:
    tilemap_base();
    ~tilemap_base() override = default;

    prop<assets::asset_ptr<material>> Material;
    prop<point_f>                     Position;

    auto add_layer(tilemap_layer const& layer) -> uid;

    auto get_tile(uid layerId, point_i pos) const -> tile_index_t;
    void set_tile(uid layerId, point_i pos, tile_index_t setIdx);

    auto is_layer_visible(uid id) const -> bool;
    void set_layer_visible(uid id, bool visible);
    auto get_layer_size(uid id) const -> size_i;

    void clear();

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

    void mark_dirty();

private:
    struct layer {
        uid     ID {INVALID_ID};
        size_i  Size {size_i::Zero};
        point_i Offset {point_i::Zero};
        i32     TileMapStart {0};
        bool    Visible {true};
    };

    auto get_layer(uid id) -> layer*;
    auto get_layer(uid id) const -> layer const*;

    void virtual setup_quad(quad& q, point_i coord, tile_index_t idx) const = 0;

    std::vector<layer>        _layers {};
    std::vector<tile_index_t> _tileMap {};

    quad_renderer                    _renderer;
    std::vector<quad>                _quads {};
    bool                             _isDirty {true};
    bool                             _updateGeometry {true};
    std::vector<std::pair<uid, i32>> _dirtyTiles;
};

////////////////////////////////////////////////////////////

template <typename G>
class tilemap : public tilemap_base {
    using grid_type = G;
    using tile_type = G::tile_type;
    using set_type  = tileset<tile_type>;

public:
    explicit tilemap(set_type set);

    prop<grid_type> Grid;

    auto get_tile_index(string const& texture) const -> tile_index_t;
    void change_tileset(tile_index_t idx, tile_type const& t);

private:
    void setup_quad(quad& q, point_i coord, tile_index_t idx) const override;

    set_type _tileSet;
};

////////////////////////////////////////////////////////////

using ortho_tilemap = tilemap<ortho_grid>;
using iso_tilemap   = tilemap<iso_grid>;
using hex_tilemap   = tilemap<hex_grid>;

}

#include "TileMap.inl"
