// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/Transformable.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

class TCOB_API shape : public transformable, public updatable {
public:
    shape();

    prop<assets::asset_ptr<material>> Material;
    prop<string>                      TextureRegion;

    prop<color>  Color;
    prop_fn<f32> Transparency;

    prop<std::optional<point_f>> Pivot;
    prop<point_f>                Center;

    void show();
    void hide();
    auto is_visible() const -> bool;

    auto virtual get_geometry() -> geometry_data = 0;

protected:
    auto get_pivot() const -> point_f override;

    void virtual on_color_changed(color c)                          = 0;
    void virtual on_center_changed(point_f center)                  = 0;
    void virtual on_texture_region_changed(string const& texRegion) = 0;

    void on_transform_changed() override;

    auto is_dirty() const -> bool;
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

    prop<f32> Radius;
    prop<i32> Segments;

    auto get_geometry() -> geometry_data override;

protected:
    void on_update(milliseconds deltaTime) override;

    void on_color_changed(color c) override;
    void on_center_changed(point_f center) override;
    void on_texture_region_changed(string const& texRegion) override;

private:
    std::vector<u32>    _indices;
    std::vector<vertex> _verts;
};

////////////////////////////////////////////////////////////

class TCOB_API rect_shape final : public shape {
public:
    rect_shape();

    prop<rect_f>  Bounds;
    prop<point_f> TextureScroll;

    auto get_geometry() -> geometry_data override;

    auto get_AABB() const -> rect_f;

    void move_by(point_f offset);

protected:
    void on_update(milliseconds deltaTime) override;

    void on_color_changed(color c) override;
    void on_center_changed(point_f center) override;
    void on_texture_region_changed(string const& texRegion) override;

private:
    void update_aabb();

    quad   _quad {};
    rect_f _aabb {rect_f::Zero};
};

////////////////////////////////////////////////////////////

class TCOB_API static_shape_batch final : public drawable {
public:
    explicit static_shape_batch(std::span<std::shared_ptr<shape>> shapees);

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    batch_polygon_renderer _renderer {};
};

////////////////////////////////////////////////////////////

class TCOB_API shape_batch final : public drawable {
public:
    shape_batch();

    template <std::derived_from<shape> T>
    auto create_shape() -> std::shared_ptr<T>
    {
        return std::static_pointer_cast<T>(_children.emplace_back(std::make_shared<T>()));
    }

    void remove_shape(shape const& shape);
    void clear();

    void move_to_front(shape const& shape);
    void send_to_back(shape const& shape);

    auto get_shape_count() const -> isize;
    auto is_empty() const -> bool;

    auto get_shape_at(usize index) const -> std::shared_ptr<shape>;

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    std::vector<std::shared_ptr<shape>> _children {};
    batch_polygon_renderer              _renderer {};
};
}
