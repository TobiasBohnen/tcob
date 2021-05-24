// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/core/ConnectionManager.hpp>
#include <tcob/core/Input.hpp>
#include <tcob/core/data/Rect.hpp>
#include <tcob/gfx/drawables/Drawable.hpp>
#include <tcob/thirdparty/sigslot/signal.hpp>

namespace tcob {
class Scene : public Drawable {
public:
    Scene(Game& game);

    void start();
    void finish();

    void wake_up();
    void sleep();

    void draw(gl::RenderTarget& target) override = 0;

protected:
    virtual void fixed_update(f64 deltaTime) = 0;

    virtual void on_start();
    virtual void on_finish();
    virtual void on_wake_up();
    virtual void on_sleep();

    virtual void on_key_down(const KeyboardEvent& ev);
    virtual void on_key_up(const KeyboardEvent& ev);
    virtual void on_mouse_motion(const MouseMotionEvent& ev);
    virtual void on_mousebutton_down(const MouseButtonEvent& ev);
    virtual void on_mousebutton_up(const MouseButtonEvent& ev);
    virtual void on_mousewheel(const MouseWheelEvent& ev);
    virtual void on_controller_axis_motion(const ControllerAxisEvent& ev);
    virtual void on_controllerbutton_down(const ControllerButtonEvent& ev);
    virtual void on_controllerbutton_up(const ControllerButtonEvent& ev);

    auto game() const -> Game&;

private:
    void attach_events();
    void detach_events();

    detail::ConnectionManager _connMan;
    Game& _game;
};
}