// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <mutex>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/RenderTarget.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API drawable : public updatable {
public:
    ~drawable() override = default;

    signal<bool const> VisibilityChanged;

    u32 VisibilityMask {0xFFFFFFFF};

    void show();
    void hide();
    auto is_visible() const -> bool;

    void draw_to(render_target& target);

    void update(milliseconds deltaTime) final;

protected:
    void virtual on_draw_to(render_target& target) = 0;
    auto virtual can_draw() const -> bool          = 0;

    void virtual on_visiblity_changed();

private:
    bool       _visible {true};
    std::mutex _mutex {};
};

////////////////////////////////////////////////////////////

class TCOB_API entity : public drawable, public input::receiver {
public:
    // TODO: bounds, mouse enter/leave
    auto virtual get_update_mode() const -> update_mode;

    void virtual fixed_update(milliseconds deltaTime);

protected:
    void on_draw_to(render_target&) override { }
    auto can_draw() const -> bool override;

    void on_update(milliseconds) override { }
    void virtual on_fixed_update(milliseconds) { }

    void on_key_down(input::keyboard::event const&) override { }
    void on_key_up(input::keyboard::event const&) override { }
    void on_text_input(input::keyboard::text_input_event const&) override { }
    void on_text_editing(input::keyboard::text_editing_event const&) override { }
    void on_mouse_motion(input::mouse::motion_event const&) override { }
    void on_mouse_button_down(input::mouse::button_event const&) override { }
    void on_mouse_button_up(input::mouse::button_event const&) override { }
    void on_mouse_wheel(input::mouse::wheel_event const&) override { }
    void on_controller_axis_motion(input::controller::axis_event const&) override { }
    void on_controller_button_down(input::controller::button_event const&) override { }
    void on_controller_button_up(input::controller::button_event const&) override { }
};

}
