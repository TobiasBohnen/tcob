// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <unordered_map>
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

enum class render_direction : u8 {
    RightDown,
    RightUp,
    LeftDown,
    LeftUp
};

////////////////////////////////////////////////////////////

using tile_index_t = u64;

template <typename T>
using tileset = std::unordered_map<tile_index_t, T>;

////////////////////////////////////////////////////////////

class TCOB_API tilemap_layer final : public non_copyable {
    friend class tilemap_base;

public:
    prop<grid<tile_index_t>> Tiles;
    prop<point_i>            Offset {point_i::Zero};
    prop<bool>               Visible {true};
    prop<render_direction>   RenderDirection {render_direction::RightDown};

protected:
    tilemap_layer(tilemap_base* parent);
    void notify_parent();

private:
    tilemap_base* _parent {nullptr};
};

////////////////////////////////////////////////////////////

class TCOB_API tilemap_base : public drawable, public updatable {
    friend class tilemap_layer;

public:
    tilemap_base();
    ~tilemap_base() override = default;

    prop<asset_ptr<material>> Material;
    prop<point_f>             Position;

    auto create_layer() -> tilemap_layer&;
    void remove_layer(tilemap_layer const& layer);
    void clear();

    void bring_to_front(tilemap_layer const& layer);
    void send_to_back(tilemap_layer const& layer);

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

    void mark_dirty();

private:
    void notify_layer_changed(tilemap_layer* layer);

    virtual void setup_quad(quad& q, point_i coord, tile_index_t idx) const = 0;

    std::vector<std::unique_ptr<tilemap_layer>> _layers {};

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

public:
    using set = tileset<tile_type>;

    tilemap();

    prop<grid_type> Grid;
    prop<set>       Tileset;

private:
    void setup_quad(quad& q, point_i coord, tile_index_t idx) const override;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API orthogonal_tile {
public:
    string TextureRegion {};
    bool   FlipHorizontally {false};
    bool   FlipVertically {false};
    color  Color {colors::White};

    size_f Scale {size_f::One};

    static auto constexpr Members();

    auto operator==(orthogonal_tile const& other) const -> bool = default;
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

    auto operator==(isometric_tile const& other) const -> bool = default;
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

    auto operator==(hexagonal_tile const& other) const -> bool = default;
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

using orthogonal_tilemap = tilemap<orthogonal_grid>;
using isometric_tilemap  = tilemap<isometric_grid>;
using hexagonal_tilemap  = tilemap<hexagonal_grid>;

////////////////////////////////////////////////////////////

}

#include "TileMap.inl"
