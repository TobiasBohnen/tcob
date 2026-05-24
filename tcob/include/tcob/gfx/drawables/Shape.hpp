// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <map>
#include <memory>
#include <optional>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/TypeFactory.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/Polygon.hpp"
#include "tcob/gfx/Ray.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/Transformable.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

class TCOB_API shape : public transformable, public updatable, public non_copyable {
public:
    shape();

    prop<asset_ptr<material>> Material;
    prop<string>              TextureRegion {"default"};

    prop<color> Color {colors::White};

    prop<std::optional<point_f>> Pivot;

    u32 IntersectMask {0xFFFFFFFF};

    void show();
    void hide();
    auto is_visible() const -> bool;

    virtual auto geometry(isize pass) -> geometry_view = 0;
    virtual auto aabb() const -> rect_f                = 0;

    virtual auto intersect(ray const& ray) const -> std::vector<ray::result> = 0;

    auto is_dirty() const -> bool;

protected:
    virtual auto center() const -> point_f = 0;
    auto         pivot() const -> point_f override;

    void on_transform_changed() override;

    void mark_dirty();
    void mark_clean();

    auto get_texture_region(pass const& pass) const -> texture_region;

private:
    bool _isDirty {false};
    bool _visible {true};
};

////////////////////////////////////////////////////////////

class TCOB_API rect_shape final : public shape {
public:
    rect_shape();

    prop<rect_f>  Bounds;
    prop<point_f> TextureScroll;

    auto geometry(isize pass) -> geometry_view override;
    auto aabb() const -> rect_f override;

    auto intersect(ray const& ray) const -> std::vector<ray::result> override;

    void move_by(point_f offset);

protected:
    void on_update(milliseconds deltaTime) override;

    auto center() const -> point_f override;

private:
    void update_geometry();

    std::unordered_map<isize, quad> _quads {};
};

////////////////////////////////////////////////////////////

class TCOB_API circle_shape final : public shape {
public:
    circle_shape();

    prop<point_f> Center;
    prop<f32>     Radius;
    prop<i32>     Segments {90};

    auto geometry(isize pass) -> geometry_view override;
    auto aabb() const -> rect_f override;

    auto intersect(ray const& ray) const -> std::vector<ray::result> override;

protected:
    void on_update(milliseconds deltaTime) override;

    auto center() const -> point_f override;

private:
    void update_geometry();

    geometry_store _store;
};

////////////////////////////////////////////////////////////

class TCOB_API poly_shape final : public shape {
public:
    poly_shape();

    prop<std::vector<polygon>> Polygons;

    auto geometry(isize pass) -> geometry_view override;
    auto aabb() const -> rect_f override;

    auto intersect(ray const& ray) const -> std::vector<ray::result> override;

    void clip(poly_shape const& other, clip_mode mode);

    void move_by(point_f offset);

protected:
    void on_update(milliseconds deltaTime) override;

    auto center() const -> point_f override;

private:
    void update_geometry();

    geometry_store _store;

    rect_f  _boundingBox {rect_f::Zero};
    point_f _centroid {point_f::Zero};
};

////////////////////////////////////////////////////////////

class TCOB_API mesh_shape final : public shape {
public:
    void set(std::span<vertex const> verts, std::span<u32 const> inds);
    auto load [[nodiscard]] (path const& file) noexcept -> bool;
    auto load [[nodiscard]] (io::istream& in, string const& ext) noexcept -> bool;

    auto geometry(isize pass) -> geometry_view override;
    auto aabb() const -> rect_f override;

    auto intersect(ray const& ray) const -> std::vector<ray::result> override;

    void move_by(point_f offset);

protected:
    void on_update(milliseconds deltaTime) override;

    auto center() const -> point_f override;

private:
    void update_geometry();

    rect_f _aabb {rect_f::Zero};

    std::vector<vertex> _verts;
    std::vector<vertex> _xformVerts;
    std::vector<u32>    _inds;

    std::map<std::pair<u32, u32>, u32> _edgeCount {};
    rect_f                             _localBounds {0, 0, 0, 0};
};

class TCOB_API mesh_loader : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<mesh_loader>> {
        static inline char const* ServiceName {"gfx::mesh_loader::factory"};
    };

    mesh_loader()          = default;
    virtual ~mesh_loader() = default;

    virtual auto load(io::istream& in) -> std::optional<geometry_store> = 0;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API shape_batch final : public drawable, public updatable {
public:
    shape_batch();

    template <DerivedFrom<shape> T>
    auto create_shape() -> T&;
    auto remove_shape(shape const& shape) -> bool;
    void clear();

    void bring_to_front(shape const& shape);
    void send_to_back(shape const& shape);
    void sort_by_y_position();

    auto size() const -> isize;
    auto is_empty() const -> bool;

    auto get_shape_at(isize index) const -> shape&;

    auto intersect(ray const& ray, u32 mask = 0xFFFFFFFF) const -> std::unordered_map<shape*, std::vector<ray::result>>;
    auto intersect(rect_f const& rect, u32 mask = 0xFFFFFFFF) const -> std::vector<shape*>;

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target, transform const& xform) override;

private:
    bool                                _isDirty {false};
    std::vector<std::unique_ptr<shape>> _children {};
    batch_renderer                      _renderer {};
};

}

#include "Shape.inl"
