// Copyright (c) 2025 Tobias Bohnen
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

    void prepare_redraw() override;

protected:
    void on_styles_changed() override;

    void on_paint(widget_painter& painter) final;
    void virtual paint_content(widget_painter& painter, rect_f const& rect) = 0;

    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_down(input::mouse::button_event const& ev) override;
    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_up(input::mouse::button_event const& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;

    void on_update(milliseconds deltaTime) override;

    void offset_content(rect_f& bounds, bool isHitTest) const override;

    auto get_scroll_max_value() const -> f32;

    auto virtual get_scroll_content_height() const -> f32 = 0;
    auto virtual get_scroll_item_count() const -> isize   = 0;

    auto get_scrollbar_value() const -> f32;
    void set_scrollbar_value(f32 value);

private:
    scrollbar _vScrollbar;
};
}
