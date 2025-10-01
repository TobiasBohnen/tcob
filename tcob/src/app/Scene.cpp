// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/app/Scene.hpp"

#include <algorithm>
#include <iterator>
#include <memory>

#include "tcob/app/Game.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/assets/AssetLibrary.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Window.hpp"

namespace tcob {
scene::scene(game& parent)
    : _game {parent}
{
}

void scene::start()
{
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
    if (_rootNode) {
        _rootNode->draw_to(target);
    }

    on_draw_to(target);
}

void scene::update(milliseconds deltaTime)
{
    if (_rootNode) {
        _rootNode->update(deltaTime);
    }

    on_update(deltaTime);
}

void scene::fixed_update(milliseconds deltaTime)
{
    if (_rootNode) {
        _rootNode->fixed_update(deltaTime);
    }

    on_fixed_update(deltaTime);
}

auto scene::root_node() -> scene_node&
{
    if (!_rootNode) {
        _rootNode = std::make_unique<scene_node>();
    }
    return *_rootNode;
}

auto scene::parent() -> game&
{
    return _game;
}

auto scene::window() -> gfx::window&
{
    return locate_service<gfx::render_system>().window();
}

auto scene::library() -> assets::library&
{
    return _game.library();
}

void scene::attach_events()
{
    _connections.connect<&scene::draw_to>(_game.Draw, this);
    _connections.connect<&scene::update>(_game.Update, this);
    _connections.connect<&scene::fixed_update>(_game.FixedUpdate, this);

    auto const& input {locate_service<input::system>()};

    _connections.connect(input.KeyUp, [this](auto&& event) { handle_input_event(event, &input::receiver::on_key_up); });
    _connections.connect(input.KeyDown, [this](auto&& event) { handle_input_event(event, &input::receiver::on_key_down); });
    _connections.connect(input.TextInput, [this](auto&& event) { handle_input_event(event, &input::receiver::on_text_input); });
    _connections.connect(input.MouseMotion, [this](auto&& event) { handle_input_event(event, &input::receiver::on_mouse_motion); });
    _connections.connect(input.MouseButtonDown, [this](auto&& event) { handle_input_event(event, &input::receiver::on_mouse_button_down); });
    _connections.connect(input.MouseButtonUp, [this](auto&& event) { handle_input_event(event, &input::receiver::on_mouse_button_up); });
    _connections.connect(input.MouseWheel, [this](auto&& event) { handle_input_event(event, &input::receiver::on_mouse_wheel); });
    _connections.connect(input.ControllerAxisMotion, [this](auto&& event) { handle_input_event(event, &input::receiver::on_controller_axis_motion); });
    _connections.connect(input.ControllerButtonDown, [this](auto&& event) { handle_input_event(event, &input::receiver::on_controller_button_down); });
    _connections.connect(input.ControllerButtonUp, [this](auto&& event) { handle_input_event(event, &input::receiver::on_controller_button_up); });
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

auto scene_node::create_child() -> scene_node&
{
    return *_children.emplace_back(std::unique_ptr<scene_node>(new scene_node {this}));
}

auto scene_node::child_count() const -> isize
{
    return std::ssize(_children);
}

auto scene_node::get_child_at(isize index) const -> scene_node&
{
    return *_children.at(static_cast<usize>(index));
}

void scene_node::clear_children()
{
    _children.clear();
}

void scene_node::bring_to_front()
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

void scene_node::on_update(milliseconds deltaTime)
{
    if (*Entity) {
        Entity->update(deltaTime);
    }

    for (auto& child : _children) {
        child->update(deltaTime);
    }
}

void scene_node::on_fixed_update(milliseconds deltaTime)
{
    if (*Entity) {
        Entity->fixed_update(deltaTime);
    }

    for (auto& child : _children) {
        child->fixed_update(deltaTime);
    }
}

void scene_node::on_draw_to(gfx::render_target& target)
{
    if (*Entity) {
        Entity->draw_to(target);
    }

    for (auto& child : _children) {
        child->draw_to(target);
    }
}

auto scene_node::can_draw() const -> bool
{
    return !_children.empty() || (*Entity && Entity->is_visible());
}

}
