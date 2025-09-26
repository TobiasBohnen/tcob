// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/RenderTarget.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API drawable {
public:
    virtual ~drawable() = default;

    signal<bool const> VisibilityChanged;

    u32 VisibilityMask {0xFFFFFFFF};

    void show();
    void hide();
    auto is_visible() const -> bool;

    void draw_to(render_target& target);

protected:
    virtual void on_draw_to(render_target& target) = 0;
    virtual auto can_draw() const -> bool          = 0;

    virtual void on_visibility_changed();

private:
    bool _visible {true};
};

////////////////////////////////////////////////////////////

class TCOB_API entity : public drawable, public hybrid_updatable, public input::receiver {
public:
    // TODO: bounds, mouse enter/leave, frame limit

    void update(milliseconds deltaTime) final;
    void fixed_update(milliseconds deltaTime) final;

protected:
    entity(update_mode mode = update_mode::Normal);

    void on_draw_to(render_target&) override { }
    auto can_draw() const -> bool override;

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
    update_mode _mode;
};

}
