// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Layout.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/component/Scrollbar.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API panel : public widget_container { // TODO: convert to template
public:
    using default_layout = static_layout;

    class TCOB_API style : public widget_style {
    public:
        scrollbar_element HScrollBar;
        scrollbar_element VScrollBar;

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    explicit panel(init const& wi);

    prop<bool> ScrollEnabled;
    prop<bool> Movable;

    template <std::derived_from<layout> T>
    auto create_layout(auto&&... args) -> T&;
    template <std::derived_from<layout> T>
    auto get_layout() -> T&;

    auto widgets() const -> std::vector<std::shared_ptr<widget>> const& override;

    void clear();

    auto scroll_offset() const -> point_f override;

protected:
    void on_prepare_redraw() override;

    void on_styles_changed() override;

    void on_draw(widget_painter& painter) override;
    void on_draw_children(widget_painter& painter) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;

    void on_update(milliseconds deltaTime) override;

    void offset_content(rect_f& bounds, bool isHitTest) const override;

    auto get_layout() const -> layout*;

    auto can_move() const -> bool;

private:
    auto requires_scroll(orientation orien, rect_f const& rect) const -> bool;
    auto get_scroll_max_value(orientation orien) const -> f32;

    std::unique_ptr<layout> _layout;
    scrollbar               _vScrollbar;
    scrollbar               _hScrollbar;

    panel::style _style;
};

////////////////////////////////////////////////////////////

class TCOB_API glass : public panel {
public:
    explicit glass(init const& wi);

protected:
    void on_draw(widget_painter& painter) override;
    void on_draw_children(widget_painter& painter) override;

    auto is_inert() const -> bool override;
};

}

#include "Panel.inl"
