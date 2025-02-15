// Copyright (c) 2025 Tobias Bohnen
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

class TCOB_API scrollbar {
public:
    struct rects {
        rect_f Bar;
        rect_f Thumb;
    };

    scrollbar(widget& parent, orientation orien);

    bool Visible {false};
    f32  Min {0.0f};
    f32  Max {0.0f};

    void update(milliseconds deltaTime);
    void paint(widget_painter& painter, element::scrollbar const& style, rect_f& rect, bool isActive);

    auto current_value() const -> f32;
    auto target_value() const -> f32;

    void start_scroll(f32 target, milliseconds delay);
    void reset();

    auto is_mouse_over() const -> bool;
    void mouse_hover(point_i mp);
    auto is_dragging() const -> bool;
    void mouse_drag(point_i mp);
    void mouse_down(point_i mp);
    void mouse_up(point_i mp);

private:
    void calculate_value(point_f mp);

    bool    _isDragging {false};
    bool    _overThumb {false};
    bool    _overBar {false};
    point_i _dragOffset {point_i::Zero};

    orientation    _orien;
    widget&        _parent;
    widget_tweener _tween;

    rects _barRectCache;

    element::scrollbar const* _style {nullptr};
};

}
