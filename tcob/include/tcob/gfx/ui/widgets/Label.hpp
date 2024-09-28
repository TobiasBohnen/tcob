// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API label : public widget {
public:
    class TCOB_API style : public background_style {
    public:
        element::text Text;
    };

    explicit label(init const& wi);

    prop<utf8_string> Label;

    std::shared_ptr<widget> For;

protected:
    void on_paint(widget_painter& painter) override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_key_up(input::keyboard::event const& ev) override;
    void on_mouse_enter() override;
    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_down(input::mouse::button_event const& ev) override;
    void on_mouse_up(input::mouse::button_event const& ev) override;
    void on_click() override;
    void on_focus_gained() override;
    void on_focus_lost() override;

    void on_update(milliseconds deltaTime) override;

    auto get_attributes() const -> widget_attributes override;

private:
    detail::input_injector _injector;
};
}
