// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/Scrollbar.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API panel : public widget_container {
public:
    class TCOB_API style : public background_style {
    public:
        element::scrollbar HScrollBar;
        element::scrollbar VScrollBar;
    };

    explicit panel(init const& wi);

    prop<bool> ScrollEnabled;

    template <std::derived_from<layout> T>
    auto create_layout(auto&&... args) -> std::shared_ptr<T>;
    template <std::derived_from<layout> T>
    auto get_layout() -> std::shared_ptr<T>;

    void force_redraw(string const& reason) override;

    auto get_widgets() const -> std::vector<std::shared_ptr<widget>> const& override;

    void clear();

    auto requires_scroll(orientation orien, rect_f const& rect) const -> bool;
    auto get_scroll_min_value(orientation orien) const -> f32;
    auto get_scroll_max_value(orientation orien) const -> f32;
    auto get_scroll_style(orientation orien) const -> element::scrollbar*;

    auto get_scroll_offset() const -> point_f override;
    void set_scroll_offset(point_f off);

protected:
    void on_styles_changed() override;

    void on_paint(widget_painter& painter) override;

    void on_mouse_hover(input::mouse::motion_event& ev) override;
    void on_mouse_drag(input::mouse::motion_event& ev) override;
    void on_mouse_down(input::mouse::button_event& ev) override;
    void on_mouse_up(input::mouse::button_event& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event& ev) override;

    void on_update(milliseconds deltaTime) override;

    void offset_content(rect_f& bounds, bool isHitTest) const override;

private:
    std::shared_ptr<layout> _layout;
    scrollbar<panel>        _vScrollbar;
    scrollbar<panel>        _hScrollbar;
};
}

#include "Panel.inl"
