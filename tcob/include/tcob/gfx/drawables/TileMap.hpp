// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <initializer_list>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Grid.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

using tile_index_t = u64;

////////////////////////////////////////////////////////////

template <typename T>
class tileset {
public:
    using tile_type = T;

    explicit tileset(std::unordered_map<tile_index_t, tile_type> set);
    explicit tileset(std::initializer_list<std::pair<tile_index_t, tile_type>> items);

    auto get_tile(tile_index_t idx) const -> tile_type const&;
    void set_tile(tile_index_t idx, tile_type const& tile_type);

private:
    std::unordered_map<tile_index_t, tile_type> _set;
};

////////////////////////////////////////////////////////////

class TCOB_API orthogonal_tile {
public:
    string TextureRegion {};
    bool   FlipHorizontally {false};
    bool   FlipVertically {false};
    color  Color {colors::White};

    size_f Scale {size_f::One};

    static auto constexpr Members();
};

class TCOB_API orthogonal_grid {
public:
    using tile_type = orthogonal_tile;

    size_f TileSize {size_f::Zero};

    auto layout_tile(orthogonal_tile const& tile, point_i coord) const -> rect_f;

    auto operator==(orthogonal_grid const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API isometric_tile {
public:
    string TextureRegion {};
    bool   FlipHorizontally {false};
    bool   FlipVertically {false};
    color  Color {colors::White};

    point_f Center {0.5f, 0.5f};
    f32     Height {0.0f};

    static auto constexpr Members();
};

class TCOB_API isometric_grid {
public:
    using tile_type = isometric_tile;

    size_f TileSize {size_f::Zero};
    bool   Staggered {false};

    auto layout_tile(isometric_tile const& tile, point_i coord) const -> rect_f;

    auto operator==(isometric_grid const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API hexagonal_tile {
public:
    string TextureRegion {};
    bool   FlipHorizontally {false};
    bool   FlipVertically {false};
    color  Color {colors::White};

    static auto constexpr Members();
};

enum hexagonal_top : u8 {
    Pointy,
    Flat
};

class TCOB_API hexagonal_grid {
public:
    using tile_type = hexagonal_tile;

    size_f        TileSize {size_f::Zero};
    hexagonal_top Top {hexagonal_top::Pointy};

    auto layout_tile(hexagonal_tile const& tile, point_i coord) const -> rect_f;

    auto operator==(hexagonal_grid const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

struct tilemap_layer {
    grid<tile_index_t> Tiles;
    point_i            Offset {point_i::Zero};
};

////////////////////////////////////////////////////////////

enum class render_direction : u8 {
    RightDown,
    RightUp,
    LeftDown,
    LeftUp
};

////////////////////////////////////////////////////////////

class TCOB_API tilemap_base : public drawable, public updatable {
public:
    tilemap_base();
    ~tilemap_base() override = default;

    prop<render_direction>    RenderDirection {render_direction::RightDown};
    prop<asset_ptr<material>> Material;
    prop<point_f>             Position;

    auto add_layer(tilemap_layer const& layer) -> uid;
    void replace_layer(uid layerId, tilemap_layer const& layer);
    void remove_layer(uid layerId);

    void bring_to_front(uid layerId);
    void send_to_back(uid layerId);

    auto is_layer_visible(uid layerId) const -> bool;
    void show_layer(uid layerId);
    void hide_layer(uid layerId);

    auto get_layer_size(uid layerId) const -> std::optional<size_i>;

    auto get_tile_index(uid layerId, point_i pos) const -> std::optional<tile_index_t>;
    void set_tile_index(uid layerId, point_i pos, tile_index_t setIdx);

    void clear();

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

    void mark_dirty();

private:
    class TCOB_API layer {
    public:
        uid     ID {INVALID_ID};
        size_i  Size {size_i::Zero};
        point_i Offset {point_i::Zero};
        i32     TileMapStart {0};
        bool    Visible {true};

        auto get_index(point_i pos) const -> i32;
    };

    auto find_layer(uid id) -> std::vector<layer>::iterator;
    auto find_layer(uid id) const -> std::vector<layer>::const_iterator;

    virtual void setup_quad(quad& q, point_i coord, tile_index_t idx) const = 0;

    std::vector<layer>        _layers {};
    std::vector<tile_index_t> _tileMap {};

    quad_renderer     _renderer;
    std::vector<quad> _quads {};
    bool              _isDirty {true};
    bool              _updateGeometry {true};
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

    auto get_tile_bounds(uid layerId, point_i pos) const -> rect_f;
    void change_tileset(tile_index_t idx, tile_type const& t);

private:
    void setup_quad(quad& q, point_i coord, tile_index_t idx) const override;

    set_type _tileSet;
};

////////////////////////////////////////////////////////////

using orthogonal_tilemap = tilemap<orthogonal_grid>;
using orthogonal_tileset = tileset<orthogonal_tile>;

using isometric_tilemap = tilemap<isometric_grid>;
using isometric_tileset = tileset<isometric_tile>;

using hexagonal_tilemap = tilemap<hexagonal_grid>;
using hexagonal_tileset = tileset<hexagonal_tile>;

////////////////////////////////////////////////////////////

}

#include "TileMap.inl"
