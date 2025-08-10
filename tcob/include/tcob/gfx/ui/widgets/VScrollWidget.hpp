// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/component/Scrollbar.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API vscroll_widget : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        scrollbar_element VScrollBar;

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

protected:
    explicit vscroll_widget(init const& wi);

    void on_styles_changed() override;

    void draw_scrollbar(widget_painter& painter, rect_f& rect);

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;

    void on_update(milliseconds deltaTime) override;

    void offset_content(rect_f& bounds, bool isHitTest) const override;

    auto virtual get_scroll_content_height() const -> f32 = 0;
    auto virtual get_scroll_distance() const -> f32       = 0;
    auto get_scroll_max() const -> f32;

    auto scrollbar_offset() const -> f32;
    void set_scrollbar_value(f32 value);

private:
    scrollbar _vScrollbar;
};
}
