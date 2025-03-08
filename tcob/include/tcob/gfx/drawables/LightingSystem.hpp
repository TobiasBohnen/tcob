// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/Polygon.hpp"
#include "tcob/gfx/Quadtree.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

class lighting_system;
class light_source;
class shadow_caster;

////////////////////////////////////////////////////////////

struct light_collision {
    point_f              Point {};
    f64                  Distance {};
    usize                CollisionCount {};
    light_source const*  Source {nullptr};
    shadow_caster const* Caster {nullptr};
};

////////////////////////////////////////////////////////////

class TCOB_API light_source {
    friend class lighting_system;

public:
    virtual ~light_source() = default;

    std::any UserData;

    prop<color>                   Color {colors::White};
    prop<point_f>                 Position;
    prop<std::optional<f32>>      Range;
    prop<bool>                    Falloff {true};
    prop<std::optional<degree_f>> StartAngle;
    prop<std::optional<degree_f>> EndAngle;

    auto is_range_limited() const -> bool;
    auto is_angle_limited() const -> bool;

protected:
    light_source(lighting_system* parent);
    void notify_parent();

private:
    lighting_system*             _parent;
    bool                         _isDirty {true};
    std::vector<light_collision> _collisionResult;
};

////////////////////////////////////////////////////////////

class TCOB_API shadow_caster {
    friend class lighting_system;

public:
    virtual ~shadow_caster() = default;

    std::any UserData;

    signal<light_collision const> Hit;
    prop<polyline>                Polygon;

protected:
    shadow_caster(lighting_system* parent);
    void notify_parent();

private:
    lighting_system* _parent;
    rect_f           _bounds;
};

////////////////////////////////////////////////////////////

class TCOB_API lighting_system final : public drawable, public updatable {
    friend class light_source;
    friend class shadow_caster;

public:
    ////////////////////////////////////////////////////////////

    explicit lighting_system(bool multiThreaded = false);
    ~lighting_system() override = default;

    prop<rect_f> Bounds;

    template <typename T = light_source>
    auto create_light_source(auto&&... args) -> std::shared_ptr<T>;

    void remove_light_source(light_source const& light);
    void clear_light_sources();

    template <typename T = shadow_caster>
    auto create_shadow_caster(auto&&... args) -> std::shared_ptr<T>;

    void remove_shadow_caster(shadow_caster const& shadow);
    void clear_shadow_casters();

    void set_blend_funcs(blend_funcs funcs);

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    void notify_light_changed(light_source* light);
    void notify_shadow_changed(shadow_caster* shadow);

    struct quadtree_node {
        rect_f               Bounds;
        shadow_caster const* Caster {nullptr};

        auto get_rect() const -> rect_f const& { return Bounds; }
        auto operator==(quadtree_node const& other) const -> bool { return Caster == other.Caster; }
    };

    struct shadow_caster_points {
        polyline_span        Points {};
        shadow_caster const* Caster {nullptr};
    };

    void rebuild_quadtree();
    void mark_lights_dirty();

    void cast_ray(light_source& light, f32 lightRange);
    void build_geometry(light_source& light, f32 lightRange, u32& indOffset);
    auto collect_angles(light_source& light, bool lightInsideShadowCaster, std::vector<shadow_caster_points> const& casterPoints) const -> std::vector<f64>;

    std::vector<std::shared_ptr<light_source>>  _lightSources {};
    std::vector<std::shared_ptr<shadow_caster>> _shadowCasters {};

    bool _isDirty {false};
    bool _updateGeometry {false};

    polygon_renderer    _renderer {buffer_usage_hint::DynamicDraw};
    std::vector<vertex> _verts;
    std::vector<u32>    _inds;

    assets::owning_asset_ptr<material> _material;

    std::mutex _mutex {};
    bool       _multiThreaded;

    std::unique_ptr<quadtree<quadtree_node>> _quadTree;
};

}

#include "LightingSystem.inl"
