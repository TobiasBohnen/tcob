// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Scrollbar.hpp"

#include <algorithm>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {

scrollbar::scrollbar(widget& parent, orientation orien)
    : _orien {orien}
    , _parent {parent}
    , _tween {parent}
{
}

void scrollbar::update(milliseconds deltaTime)
{
    _tween.update(deltaTime);
}

void scrollbar::paint(widget_painter& painter, element::scrollbar const& scrollbar, element::thumb const& thumb, rect_f& rect)
{
    _delay = scrollbar.Bar.Delay;

    if (!Visible) { return; }

    i32 const numBlocks {10};
    f32 const frac {current_value()};

    element::bar::context const barCtx {
        .Orientation = _orien,
        .Inverted    = _orien == orientation::Vertical,
        .Position    = element::bar::position::RightOrBottom,
        .BlockCount  = numBlocks,
        .Fraction    = frac};
    element::thumb::context const thumbCtx {
        .Orientation = barCtx.Orientation,
        .Inverted    = barCtx.Inverted,
        .Fraction    = barCtx.Fraction};

    rect_f const scrRect {rect};
    _barRectCache.Bar   = painter.draw_bar(scrollbar.Bar, scrRect, barCtx);
    _barRectCache.Thumb = painter.draw_thumb(thumb, _barRectCache.Bar, thumbCtx);

    if (_orien == orientation::Vertical) {
        rect.Size.Width -= _barRectCache.Bar.width() + scrollbar.Bar.Border.Size.calc(_barRectCache.Bar.width());
    } else if (_orien == orientation::Horizontal) {
        rect.Size.Height -= _barRectCache.Bar.height() + scrollbar.Bar.Border.Size.calc(_barRectCache.Bar.height());
    }
}

auto scrollbar::is_mouse_over() const -> bool
{
    return Visible && (_overBar || _overThumb);
}

auto scrollbar::is_mouse_over_thumb() const -> bool
{
    return Visible && _overThumb;
}

void scrollbar::mouse_hover(point_i mp)
{
    if (_isDragging) { return; }

    _overThumb = false;
    _overBar   = false;

    if (!Visible) { return; }

    if (_barRectCache.Thumb.contains(_parent.global_to_local(mp))) {
        _overThumb = true;
        return;
    }

    // over bar
    if (_barRectCache.Bar.contains(_parent.global_to_local(mp))) {
        _overBar = true;
        return;
    }
}

auto scrollbar::is_dragging() const -> bool
{
    return _isDragging;
}

void scrollbar::mouse_drag(point_i mp)
{
    if (_isDragging || is_mouse_over()) {
        calculate_value(_parent.global_to_content(mp));
        _isDragging = true;
    }
}

void scrollbar::mouse_up(point_i mp)
{
    _dragOffset = point_i::Zero;
    _isDragging = false;

    _overThumb = _barRectCache.Thumb.contains(_parent.global_to_local(mp));
    _overBar   = _barRectCache.Bar.contains(_parent.global_to_local(mp));
}

void scrollbar::mouse_leave()
{
    _dragOffset = point_i::Zero;
    _isDragging = false;

    _overThumb = false;
    _overBar   = false;
}

void scrollbar::mouse_down(point_i mp)
{
    _isDragging = false;

    if (!is_mouse_over()) { return; }
    if (!_overThumb) {
        calculate_value(_parent.global_to_content(mp));
    } else {
        _dragOffset = point_i {_parent.global_to_local(mp) - _barRectCache.Thumb.center()};
        _isDragging = true;
    }
}

auto scrollbar::current_value() const -> f32
{
    return _tween.current_value();
}

auto scrollbar::target_value() const -> f32
{
    return _tween.target_value();
}

void scrollbar::start_scroll(f32 target, milliseconds delay)
{
    if (!Visible) {
        _tween.reset(target);
        return;
    }

    f32 const actTarget {std::clamp(target, 0.0f, 1.0f)};
    if (!_isDragging) {
        _tween.start(actTarget, delay);
    } else {
        _tween.reset(actTarget);
    }
}

void scrollbar::reset()
{
    _tween.reset(0);
    _delay = milliseconds {0};
}

void scrollbar::calculate_value(point_f mp)
{
    rect_f const rect {_barRectCache.Bar};
    f32          frac {0.0f};

    switch (_orien) {
    case orientation::Horizontal: {
        f32 const tw {_barRectCache.Thumb.width()};
        frac = (mp.X - _dragOffset.X - (tw / 2)) / (rect.width() - tw);
    } break;
    case orientation::Vertical: {
        f32 const th {_barRectCache.Thumb.height()};
        frac = (mp.Y - _dragOffset.Y - (th / 2)) / (rect.height() - th);
    } break;
    }

    start_scroll(frac, _delay);

    _overThumb = true;
}

} // namespace ui
