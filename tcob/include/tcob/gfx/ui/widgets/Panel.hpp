// Copyright (c) 2025 Tobias Bohnen
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
    class TCOB_API style : public widget_style {
    public:
        element::scrollbar HScrollBar;
        element::scrollbar VScrollBar;

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    explicit panel(init const& wi);

    prop<bool> ScrollEnabled;

    template <std::derived_from<layout> T>
    auto create_layout(auto&&... args) -> std::shared_ptr<T>;
    template <std::derived_from<layout> T>
    auto get_layout() -> std::shared_ptr<T>;

    void force_redraw(string const& reason) override;

    auto widgets() const -> std::vector<std::shared_ptr<widget>> const& override;

    void clear_widgets();

    auto scroll_offset() const -> point_f override;

protected:
    void on_styles_changed() override;

    void on_paint(widget_painter& painter) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_down(input::mouse::button_event const& ev) override;
    void on_mouse_up(input::mouse::button_event const& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;

    void on_update(milliseconds deltaTime) override;

    void offset_content(rect_f& bounds, bool isHitTest) const override;

    auto get_layout() -> std::shared_ptr<layout>;

private:
    auto requires_scroll(orientation orien, rect_f const& rect) const -> bool;
    auto get_scroll_max_value(orientation orien) const -> f32;

    std::shared_ptr<layout> _layout;
    scrollbar               _vScrollbar;
    scrollbar               _hScrollbar;

    panel::style _style;
};

////////////////////////////////////////////////////////////

class TCOB_API glass : public panel {
public:
    explicit glass(init const& wi);

protected:
    void on_paint(widget_painter& painter) override;

    auto is_inert() const -> bool override;
};

}

#include "Panel.inl"
