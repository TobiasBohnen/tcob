// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/game/Scene.hpp>

#include <tcob/game/Game.hpp>

namespace tcob {
Scene::Scene(Game& game)
    : _game { game }
{
}

void Scene::start()
{
    attach_events();
    on_start();
}

void Scene::finish()
{
    detach_events();
    on_finish();
}

void Scene::wake_up()
{
    attach_events();
    on_wake_up();
}

void Scene::sleep()
{
    detach_events();
    on_sleep();
}

auto Scene::game() const -> Game&
{
    return _game;
}

void Scene::attach_events()
{
    _connMan.connect(_game.Draw, &Scene::draw, this);
    _connMan.connect(_game.Update, &Scene::update, this);
    _connMan.connect(_game.FixedUpdate, &Scene::fixed_update, this);

    auto& input { _game.input() };
    _connMan.connect(input.KeyUp, &Scene::on_key_up, this);
    _connMan.connect(input.KeyDown, &Scene::on_key_down, this);
    _connMan.connect(input.MouseMotion, &Scene::on_mouse_motion, this);
    _connMan.connect(input.MouseButtonDown, &Scene::on_mousebutton_down, this);
    _connMan.connect(input.MouseButtonUp, &Scene::on_mousebutton_up, this);
    _connMan.connect(input.MouseWheel, &Scene::on_mousewheel, this);
    _connMan.connect(input.ControllerAxisMotion, &Scene::on_controller_axis_motion, this);
    _connMan.connect(input.ControllerButtonDown, &Scene::on_controllerbutton_down, this);
    _connMan.connect(input.ControllerButtonUp, &Scene::on_controllerbutton_up, this);
}

void Scene::detach_events()
{
    _connMan.disconnect_all();
}

void Scene::on_start()
{
}

void Scene::on_finish()
{
}

void Scene::on_wake_up()
{
}

void Scene::on_sleep()
{
}

void Scene::on_key_down(const KeyboardEvent&)
{
}

void Scene::on_key_up(const KeyboardEvent&)
{
}

void Scene::on_mouse_motion(const MouseMotionEvent&)
{
}

void Scene::on_mousebutton_down(const MouseButtonEvent&)
{
}

void Scene::on_mousebutton_up(const MouseButtonEvent&)
{
}

void Scene::on_mousewheel(const MouseWheelEvent&)
{
}

void Scene::on_controller_axis_motion(const ControllerAxisEvent&)
{
}

void Scene::on_controllerbutton_down(const ControllerButtonEvent&)
{
}

void Scene::on_controllerbutton_up(const ControllerButtonEvent&)
{
}
}