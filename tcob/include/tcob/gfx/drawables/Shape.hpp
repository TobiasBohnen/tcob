// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <memory>
#include <optional>
#include <span>
#include <unordered_map>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/Polygon.hpp"
#include "tcob/gfx/Ray.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/Transformable.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

class TCOB_API shape : public transformable, public updatable {
public:
    shape();

    prop<assets::asset_ptr<material>> Material;
    prop<string>                      TextureRegion {"default"};

    prop<color>  Color {colors::White};
    prop_fn<f32> Transparency;

    prop<std::optional<point_f>> Pivot;

    std::any UserData;

    u32 RayCastMask {0xFFFFFFFF};

    void show();
    void hide();
    auto is_visible() const -> bool;

    auto virtual geometry() -> geometry_data = 0;

    auto virtual intersect(ray const& ray) -> std::vector<ray::result> = 0;

    auto is_dirty() const -> bool;

protected:
    auto virtual center() const -> point_f = 0;
    auto pivot() const -> point_f override;

    void virtual on_color_changed(color c)                          = 0;
    void virtual on_texture_region_changed(string const& texRegion) = 0;

    void on_transform_changed() override;

    void mark_dirty();
    void mark_clean();

private:
    bool _isDirty {false};
    bool _visible {true};
};

////////////////////////////////////////////////////////////

class TCOB_API circle_shape : public shape {
public:
    circle_shape();

    prop<point_f> Center;
    prop<f32>     Radius;
    prop<i32>     Segments {90};

    auto geometry() -> geometry_data override;

    auto intersect(ray const& ray) -> std::vector<ray::result> override;

protected:
    void on_update(milliseconds deltaTime) override;

    auto center() const -> point_f override;
    void on_color_changed(color c) override;
    void on_texture_region_changed(string const& texRegion) override;

private:
    void create();

    std::vector<u32>    _inds;
    std::vector<vertex> _verts;
};

////////////////////////////////////////////////////////////

class TCOB_API rect_shape final : public shape {
public:
    rect_shape();

    prop<rect_f>  Bounds;
    prop<point_f> TextureScroll;

    auto geometry() -> geometry_data override;

    auto intersect(ray const& ray) -> std::vector<ray::result> override;

    auto aabb() const -> rect_f;

    void move_by(point_f offset);

protected:
    void on_update(milliseconds deltaTime) override;

    auto center() const -> point_f override;
    void on_color_changed(color c) override;
    void on_texture_region_changed(string const& texRegion) override;

private:
    void update_aabb();

    quad   _quad {};
    rect_f _aabb {rect_f::Zero};
};

////////////////////////////////////////////////////////////

class TCOB_API poly_shape final : public shape {
public:
    poly_shape();

    prop<std::vector<polygon>> Polygons;

    auto geometry() -> geometry_data override;

    auto intersect(ray const& ray) -> std::vector<ray::result> override;

    void clip(poly_shape const& other, clip_mode mode);

    void move_by(point_f offset);

protected:
    void on_update(milliseconds deltaTime) override;

    auto center() const -> point_f override;
    void on_color_changed(color c) override;
    void on_texture_region_changed(string const& texRegion) override;

private:
    void create();

    std::vector<u32>    _inds;
    std::vector<vertex> _verts;

    rect_f  _boundingBox;
    point_f _centroid;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API static_shape_batch final : public drawable {
public:
    explicit static_shape_batch(std::span<std::shared_ptr<shape>> shapes);

protected:
    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    batch_polygon_renderer _renderer {};
};

////////////////////////////////////////////////////////////

class TCOB_API shape_batch final : public drawable, public updatable {
public:
    shape_batch();

    template <std::derived_from<shape> T>
    auto create_shape() -> std::shared_ptr<T>;
    template <std::derived_from<shape> T>
    void add_shape(std::shared_ptr<T> const& shape);

    void remove_shape(shape const& shape);
    void clear();

    void bring_to_front(shape const& shape);
    void send_to_back(shape const& shape);

    auto shape_count() const -> isize;
    auto is_empty() const -> bool;

    auto get_shape_at(isize index) const -> std::shared_ptr<shape>;

    auto intersect(ray const& ray, u32 mask = 0xFFFFFFFF) -> std::unordered_map<shape*, std::vector<ray::result>>;

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    bool                                _isDirty {false};
    std::vector<std::shared_ptr<shape>> _children {};
    batch_polygon_renderer              _renderer {};
};

}

#include "Shape.inl"
