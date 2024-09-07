// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/WidgetTweener.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

template <typename Parent>
class scrollbar {
public:
    scrollbar(Parent& parent, orientation orien);

    bool Visible {false};

    void update(milliseconds deltaTime);
    void paint(widget_painter& painter, element::scrollbar const& style, rect_f& rect, bool isActive);

    auto get_current_value() const -> f32;
    auto get_target_value() const -> f32;
    void set_target_value(f32 val, milliseconds delay);

    auto is_mouse_over() const -> bool;
    auto inject_mouse_hover(point_i mp) -> bool;
    auto inject_mouse_drag(point_i mp) -> bool;
    void inject_mouse_down(point_i mp);
    void inject_mouse_up(point_i mp);

private:
    auto get_min_value() const -> f32;
    auto get_max_value() const -> f32;
    auto requires_scroll(rect_f const& rect) const -> bool;
    void calculate_value(point_f mp);

    auto get_thumb_style(widget_flags flags) -> thumb_style*;

    bool    _isDragging {false};
    bool    _overThumb {false};
    bool    _overBar {false};
    point_i _dragOffset {point_i::Zero};

    orientation               _orien;
    Parent&                   _parent;
    widget_tweener            _tween;
    element::scrollbar::rects _paintResult;
};

}

#include "Scrollbar.inl"
