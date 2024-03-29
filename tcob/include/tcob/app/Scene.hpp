// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Window.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

class game;

////////////////////////////////////////////////////////////

class TCOB_API scene_node : public gfx::drawable {
    friend class scene;

public:
    scene_node();

    auto create_child() -> std::shared_ptr<scene_node>;
    auto get_child_count() const -> isize;
    auto get_child_at(usize index) const -> std::shared_ptr<scene_node>;
    void clear_children();

    auto get_entity() const -> std::shared_ptr<gfx::entity>;
    void attach_entity(std::shared_ptr<gfx::entity> ent);

    void move_to_front();
    void send_to_back();

    void fixed_update(milliseconds deltaTime);

protected:
    explicit scene_node(scene_node* parent);

    void on_update(milliseconds deltaTime) override;

    void on_draw_to(gfx::render_target& target) override;
    auto can_draw() const -> bool override;

private:
    void move_child_to_front(scene_node* node);
    void send_child_to_back(scene_node* node);

    void handle_input_event(auto&& event, auto&& handler);

    std::shared_ptr<gfx::entity>             _entity;
    std::vector<std::shared_ptr<scene_node>> _children;
    scene_node*                              _parent;
};

////////////////////////////////////////////////////////////

class TCOB_API scene : public updatable, public input::receiver {
public:
    scene(game& parent);

    void start();
    void finish();

    void wake_up();
    void sleep();

    void draw_to(gfx::render_target& target);

    void update(milliseconds deltaTime) final;
    void fixed_update(milliseconds deltaTime);

    auto get_root_node() -> std::shared_ptr<scene_node>;

protected:
    auto get_game() -> game&;
    auto get_window() -> gfx::window&;

    void virtual on_fixed_update(milliseconds /* deltaTime */) { }

    void virtual on_start();
    void virtual on_finish();
    void virtual on_wake_up();
    void virtual on_sleep();

    void virtual on_draw_to(gfx::render_target& target) = 0;

    void on_key_down(input::keyboard::event&) override { }
    void on_key_up(input::keyboard::event&) override { }
    void on_text_input(input::keyboard::text_input_event&) override { }
    void on_text_editing(input::keyboard::text_editing_event&) override { }
    void on_mouse_motion(input::mouse::motion_event&) override { }
    void on_mouse_button_down(input::mouse::button_event&) override { }
    void on_mouse_button_up(input::mouse::button_event&) override { }
    void on_mouse_wheel(input::mouse::wheel_event&) override { }
    void on_controller_axis_motion(input::controller::axis_event&) override { }
    void on_controller_button_down(input::controller::button_event&) override { }
    void on_controller_button_up(input::controller::button_event&) override { }

private:
    void attach_events();
    void detach_events();
    void handle_input_event(auto&& event, auto&& handler);

    detail::connection_manager  _connections {};
    game&                       _game;
    std::shared_ptr<scene_node> _rootNode;
};

}

#include "Scene.inl"
