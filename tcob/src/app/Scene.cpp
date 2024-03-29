// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/app/Scene.hpp"

#include <algorithm>
#include <utility>

#include "tcob/app/Game.hpp"
#include "tcob/core/ServiceLocator.hpp"

namespace tcob {
scene::scene(game& parent)
    : _game {parent}
{
}

void scene::start()
{
    _rootNode = std::make_shared<scene_node>();

    attach_events();
    on_start();
}

void scene::finish()
{
    _rootNode = nullptr;

    detach_events();
    on_finish();
}

void scene::wake_up()
{
    attach_events();
    on_wake_up();
}

void scene::sleep()
{
    detach_events();
    on_sleep();
}

void scene::draw_to(gfx::render_target& target)
{
    _rootNode->draw_to(target);

    on_draw_to(target);
}

void scene::update(milliseconds deltaTime)
{
    _rootNode->update(deltaTime);

    on_update(deltaTime);
}

void scene::fixed_update(milliseconds deltaTime)
{
    _rootNode->fixed_update(deltaTime);

    on_fixed_update(deltaTime);
}

auto scene::get_root_node() -> std::shared_ptr<scene_node>
{
    return _rootNode;
}

auto scene::get_game() -> game&
{
    return _game;
}

auto scene::get_window() -> gfx::window&
{
    return _game.get_window();
}

void scene::attach_events()
{
    _connections.connect<&scene::draw_to>(_game.Draw, this);
    _connections.connect<&scene::update>(_game.Update, this);
    _connections.connect<&scene::fixed_update>(_game.FixedUpdate, this);

    auto const& input {locate_service<input::system>()};

    _connections.connect(input.KeyUp, [&](auto&& event) { handle_input_event(event, &input::receiver::on_key_up); });
    _connections.connect(input.KeyDown, [&](auto&& event) { handle_input_event(event, &input::receiver::on_key_down); });
    _connections.connect(input.TextInput, [&](auto&& event) { handle_input_event(event, &input::receiver::on_text_input); });
    _connections.connect(input.TextEditing, [&](auto&& event) { handle_input_event(event, &input::receiver::on_text_editing); });
    _connections.connect(input.MouseMotion, [&](auto&& event) { handle_input_event(event, &input::receiver::on_mouse_motion); });
    _connections.connect(input.MouseButtonDown, [&](auto&& event) { handle_input_event(event, &input::receiver::on_mouse_button_down); });
    _connections.connect(input.MouseButtonUp, [&](auto&& event) { handle_input_event(event, &input::receiver::on_mouse_button_up); });
    _connections.connect(input.MouseWheel, [&](auto&& event) { handle_input_event(event, &input::receiver::on_mouse_wheel); });
    _connections.connect(input.ControllerAxisMotion, [&](auto&& event) { handle_input_event(event, &input::receiver::on_controller_axis_motion); });
    _connections.connect(input.ControllerButtonDown, [&](auto&& event) { handle_input_event(event, &input::receiver::on_controller_button_down); });
    _connections.connect(input.ControllerButtonUp, [&](auto&& event) { handle_input_event(event, &input::receiver::on_controller_button_up); });
}

void scene::detach_events()
{
    _connections.disconnect_all();
}

void scene::on_start()
{
}

void scene::on_finish()
{
}

void scene::on_wake_up()
{
}

void scene::on_sleep()
{
}

////////////////////////////////////////////////////////////

scene_node::scene_node()
    : _parent {nullptr}
{
}

scene_node::scene_node(scene_node* parent)
    : _parent {parent}
{
}

auto scene_node::create_child() -> std::shared_ptr<scene_node>
{
    return _children.emplace_back(std::shared_ptr<scene_node>(new scene_node {this}));
}

auto scene_node::get_child_count() const -> isize
{
    return std::ssize(_children);
}

auto scene_node::get_child_at(usize index) const -> std::shared_ptr<scene_node>
{
    return _children.at(index);
}

void scene_node::clear_children()
{
    _children.clear();
}

auto scene_node::get_entity() const -> std::shared_ptr<gfx::entity>
{
    return _entity;
}

void scene_node::attach_entity(std::shared_ptr<gfx::entity> ent)
{
    _entity = std::move(ent);
}

void scene_node::move_to_front()
{
    if (_parent) { _parent->move_child_to_front(this); }
}

void scene_node::move_child_to_front(scene_node* node)
{
    auto it {std::ranges::find_if(_children, [node](auto& ptr) { return ptr.get() == node; })};
    if (it != _children.end()) {
        std::rotate(it, it + 1, _children.end());
    }
}

void scene_node::send_to_back()
{
    if (_parent) { _parent->send_child_to_back(this); }
}

void scene_node::send_child_to_back(scene_node* node)
{
    auto it {std::ranges::find_if(_children, [node](auto& ptr) { return ptr.get() == node; })};
    if (it != _children.end()) {
        std::rotate(_children.begin(), it, it + 1);
    }
}

void scene_node::fixed_update(milliseconds deltaTime)
{
    if (_entity && (_entity->get_update_mode() == update_mode::Fixed || _entity->get_update_mode() == update_mode::Both)) {
        _entity->fixed_update(deltaTime);
    }

    for (auto& child : _children) {
        child->fixed_update(deltaTime);
    }
}

void scene_node::on_update(milliseconds deltaTime)
{
    if (_entity && (_entity->get_update_mode() == update_mode::Normal || _entity->get_update_mode() == update_mode::Both)) {
        _entity->update(deltaTime);
    }

    for (auto& child : _children) {
        child->update(deltaTime);
    }
}

void scene_node::on_draw_to(gfx::render_target& target)
{
    if (_entity) {
        _entity->draw_to(target);
    }

    for (auto& child : _children) {
        child->draw_to(target);
    }
}

auto scene_node::can_draw() const -> bool
{
    return _entity && _entity->is_visible();
}

}
