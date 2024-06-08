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

class TCOB_API sprite final : public rect_transformable, public updatable {
    friend class sprite_batch;
    friend class static_sprite_batch;

public:
    sprite();

    prop<assets::asset_ptr<material>> Material;
    prop<color>                       Color;
    prop<string>                      TextureRegion;
    prop<point_f>                     TextureScroll;
    prop_fn<f32>                      Transparency;

    auto get_AABB() const -> rect_f;

    void show();
    void hide();
    auto is_visible() const -> bool;

protected:
    void on_update(milliseconds deltaTime) override;

    void on_transform_dirty() override;

private:
    void update_aabb();

    quad   _quad {};
    rect_f _aabb {rect_f::Zero};

    bool _visible {true};
    bool _isDirty {true};
};

////////////////////////////////////////////////////////////

class TCOB_API static_sprite_batch final : public drawable {
public:
    explicit static_sprite_batch(std::span<std::shared_ptr<sprite>> sprites);

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    batch_quad_renderer _renderer {};
};

////////////////////////////////////////////////////////////

class TCOB_API sprite_batch final : public drawable {
public:
    sprite_batch();

    auto create_sprite() -> std::shared_ptr<sprite>;
    void remove_sprite(std::shared_ptr<sprite> const& sprite);
    void clear();

    void move_to_front(std::shared_ptr<sprite> const& sprite);
    void send_to_back(std::shared_ptr<sprite> const& sprite);

    auto get_sprite_count() const -> isize;
    auto get_sprite_at(usize index) const -> std::shared_ptr<sprite>;

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    std::vector<std::shared_ptr<sprite>> _children {};
    batch_quad_renderer                  _renderer {};
};
}
