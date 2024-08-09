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

class TCOB_API mesh : public updatable {
public:
    prop<assets::asset_ptr<material>> Material;

    void show();
    void hide();
    auto is_visible() const -> bool;

    auto virtual get_geometry() -> geometry_data = 0;

private:
    bool _visible {true};
};

////////////////////////////////////////////////////////////

class TCOB_API sprite final : public rect_transformable, public mesh {
public:
    sprite();

    prop<color>   Color;
    prop<string>  TextureRegion;
    prop<point_f> TextureScroll;
    prop_fn<f32>  Transparency;

    auto get_geometry() -> geometry_data override;

    auto get_AABB() const -> rect_f;

protected:
    void on_update(milliseconds deltaTime) override;

    void on_transform_dirty() override;

private:
    void update_aabb();

    quad   _quad {};
    rect_f _aabb {rect_f::Zero};

    bool _isDirty {true};
};

////////////////////////////////////////////////////////////

class TCOB_API static_mesh_batch final : public drawable {
public:
    explicit static_mesh_batch(std::span<std::shared_ptr<mesh>> meshes);

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    batch_polygon_renderer _renderer {};
};

////////////////////////////////////////////////////////////

class TCOB_API mesh_batch final : public drawable {
public:
    mesh_batch();

    template <std::derived_from<mesh> T>
    auto create_mesh() -> std::shared_ptr<T>
    {
        return std::static_pointer_cast<T>(_children.emplace_back(std::make_shared<T>()));
    }

    void remove_mesh(mesh const& mesh);
    void clear();

    void move_to_front(mesh const& mesh);
    void send_to_back(mesh const& mesh);

    auto get_mesh_count() const -> isize;
    auto is_empty() const -> bool;

    auto get_mesh_at(usize index) const -> std::shared_ptr<mesh>;

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    std::vector<std::shared_ptr<mesh>> _children {};
    batch_polygon_renderer             _renderer {};
};
}
