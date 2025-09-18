// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/component/WidgetTweener.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API scrollbar {
public:
    struct rects {
        rect_f Bar;
        rect_f Thumb;
    };

    scrollbar(orientation orien);

    signal<> ValueChanged;

    bool Visible {false};

    void update(milliseconds deltaTime);
    void draw(widget_painter& painter, scrollbar_element const& scrollbar, thumb_element const& thumb, rect_f& rect);

    auto current_value() const -> f32;

    void start(f32 target);
    void reset(f32 target);

    auto is_mouse_over() const -> bool;
    auto is_mouse_over_thumb() const -> bool;

    auto mouse_hover(widget const& widget, point_i mp) -> bool;
    auto mouse_drag(widget const& widget, point_i mp) -> bool;
    void mouse_down(widget const& widget, point_i mp);
    void mouse_up(widget const& widget, point_i mp);
    void mouse_leave();

private:
    void do_start(f32 target);

    void calculate_value(point_f mp);

    bool    _isDragging {false};
    bool    _overThumb {false};
    bool    _overBar {false};
    point_f _dragOffset {point_f::Zero};

    orientation    _orien;
    widget_tweener _tween;

    rects _barRectCache;

    milliseconds _delay {0};
};

}
