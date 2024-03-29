// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/Scrollbar.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API vscroll_widget : public widget {
public:
    class TCOB_API style : public background_style {
    public:
        element::scrollbar VScrollBar;
    };

    explicit vscroll_widget(init const& wi);

    auto requires_scroll(orientation orien, rect_f const& rect) const -> bool;
    auto get_scroll_min_value(orientation orien) const -> f32;
    auto get_scroll_max_value(orientation orien) const -> f32;
    auto get_scroll_style(orientation orien) const -> element::scrollbar*;

protected:
    void on_styles_changed() override;

    void on_paint(widget_painter& painter) final;
    void virtual paint_content(widget_painter& painter, rect_f const& rect) = 0;

    void on_mouse_hover(input::mouse::motion_event& ev) override;
    void on_mouse_down(input::mouse::button_event& ev) override;
    void on_mouse_drag(input::mouse::motion_event& ev) override;
    void on_mouse_up(input::mouse::button_event& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event& ev) override;

    void on_update(milliseconds deltaTime) override;

    void offset_content(rect_f& bounds, bool isHitTest) const override;

    auto virtual get_list_height() const -> f32       = 0;
    auto virtual get_list_item_count() const -> isize = 0;
    auto get_scrollbar_value() const -> f32;
    void set_scrollbar_value(f32 value);

private:
    scrollbar<vscroll_widget> _vScrollbar;
};
}
