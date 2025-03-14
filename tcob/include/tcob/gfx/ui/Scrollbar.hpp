// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/WidgetTweener.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API scrollbar {
public:
    struct rects {
        rect_f Bar;
        rect_f Thumb;
    };

    scrollbar(widget& parent, orientation orien);

    signal<> Changed;

    bool Visible {false};

    void update(milliseconds deltaTime);
    void paint(widget_painter& painter, scrollbar_element const& scrollbar, thumb_element const& thumb, rect_f& rect);

    auto current_value() const -> f32;
    auto target_value() const -> f32;

    void start_scroll(f32 target, milliseconds delay);
    void reset();

    auto is_mouse_over() const -> bool;
    auto is_mouse_over_thumb() const -> bool;
    void mouse_hover(point_i mp);

    auto is_dragging() const -> bool;
    void mouse_drag(point_i mp);

    void mouse_down(point_i mp);
    void mouse_up(point_i mp);

    void mouse_leave();

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

    milliseconds _delay {0};
};

}
