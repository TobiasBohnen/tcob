// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

class lighting_system;

////////////////////////////////////////////////////////////

class TCOB_API light_source {
    friend class lighting_system;

public:
    virtual ~light_source() = default;

    prop<color>                   Color {colors::White};
    prop<point_f>                 Position;
    prop<std::optional<f32>>      Range;
    prop<std::optional<degree_f>> StartAngle;
    prop<std::optional<degree_f>> EndAngle;

protected:
    light_source(lighting_system* parent);
    void request_redraw();

private:
    lighting_system* _parent;
};

////////////////////////////////////////////////////////////

class TCOB_API shadow_caster {
    friend class lighting_system;

public:
    virtual ~shadow_caster() = default;

    prop<std::vector<point_f>> Points;

protected:
    shadow_caster(lighting_system* parent);
    void request_redraw();

private:
    lighting_system* _parent;
};

////////////////////////////////////////////////////////////

class TCOB_API lighting_system final : public drawable {
public:
    lighting_system();
    ~lighting_system() override = default;

    prop<rect_f> Bounds;

    template <typename T = light_source>
    auto create_light_source(auto&&... args) -> std::shared_ptr<T>;

    void remove_light_source(light_source const& emitter);
    void clear_light_sources();

    template <typename T = shadow_caster>
    auto create_shadow_caster(auto&&... args) -> std::shared_ptr<T>;

    void remove_shadow_caster(shadow_caster const& emitter);
    void clear_shadow_casters();

    void request_redraw();

    void set_blend_funcs(blend_funcs funcs);

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    auto ray_intersects_polygon(point_f rayOrigin, f32 rayDirection, std::span<point_f const> polygon) const -> std::vector<point_f>;

    std::vector<std::shared_ptr<light_source>>  _lightSources {};
    std::vector<std::shared_ptr<shadow_caster>> _shadowCasters {};

    bool _isDirty {false};
    bool _updateGeometry {false};

    polygon_renderer    _renderer {buffer_usage_hint::DynamicDraw};
    std::vector<vertex> _verts;
    std::vector<u32>    _inds;

    assets::manual_asset_ptr<material> _material;
};

}

#include "LightingSystem.inl"
