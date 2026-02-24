// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <vector>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/assets/Assets.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Transformable.hpp"
#include "tcob/gfx/Window.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

class game;

////////////////////////////////////////////////////////////

class TCOB_API scene_node : public gfx::drawable, public gfx::transformable, public hybrid_updatable {
    friend class scene;

public:
    scene_node();

    prop<std::shared_ptr<gfx::entity>> Entity;

    auto create_child() -> scene_node&;
    auto remove_child(scene_node const& node) -> bool;

    auto child_count() const -> isize;
    auto get_child_at(isize index) const -> scene_node&;
    void clear();

    void bring_to_front();
    void send_to_back();

    auto world_transform() const -> transform;

protected:
    explicit scene_node(scene_node* parent);

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(gfx::render_target& target, transform const& xform) override;

    auto pivot() const -> point_f override;
    void on_transform_changed() override { }

private:
    void move_child_to_front(scene_node* node);
    void send_child_to_back(scene_node* node);

    void handle_input_event(auto&& event, auto&& handler, transform const& xform);

    std::vector<std::unique_ptr<scene_node>> _children;
    scene_node*                              _parent;
};

////////////////////////////////////////////////////////////

class TCOB_API scene : public hybrid_updatable, public input::receiver {
public:
    scene(game& parent);

    void start();
    void finish();

    void wake_up();
    void sleep();

    void draw_to(gfx::render_target& target);

    void update(milliseconds deltaTime) final;
    void fixed_update(milliseconds deltaTime) final;

    auto root_node() -> scene_node&;

protected:
    auto parent() -> game&;
    auto window() -> gfx::window&;
    auto library() -> assets::library&;

    virtual void on_start();
    virtual void on_finish();
    virtual void on_wake_up();
    virtual void on_sleep();

    virtual void on_draw_to(gfx::render_target& target, transform const& xform) = 0;

    void on_key_down(input::keyboard::event const&) override { }
    void on_key_up(input::keyboard::event const&) override { }
    void on_text_input(input::keyboard::text_input_event const&) override { }
    void on_mouse_motion(input::mouse::motion_event const&) override { }
    void on_mouse_button_down(input::mouse::button_event const&) override { }
    void on_mouse_button_up(input::mouse::button_event const&) override { }
    void on_mouse_wheel(input::mouse::wheel_event const&) override { }
    void on_controller_axis_motion(input::controller::axis_event const&) override { }
    void on_controller_button_down(input::controller::button_event const&) override { }
    void on_controller_button_up(input::controller::button_event const&) override { }

private:
    void attach_events();
    void detach_events();
    void handle_input_event(auto&& event, auto&& handler, transform const& xform);

    detail::connection_manager  _connections {};
    game&                       _game;
    std::unique_ptr<scene_node> _rootNode;
};

}

#include "Scene.inl"
