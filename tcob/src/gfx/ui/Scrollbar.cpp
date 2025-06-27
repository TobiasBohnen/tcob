// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Scrollbar.hpp"

#include <algorithm>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::ui {

scrollbar::scrollbar(orientation orien)
    : _orien {orien}
{
    _tween.Changed.connect([this]() { Changed(); });
}

void scrollbar::update(milliseconds deltaTime)
{
    _tween.update(deltaTime);
}

void scrollbar::draw(widget_painter& painter, scrollbar_element const& scrollbar, thumb_element const& thumb, rect_f& rect)
{
    _delay = scrollbar.Bar.MotionDuration;

    if (!Visible) { return; }

    bar_element::context const barCtx {
        .Orientation = _orien,
        .Position    = bar_element::position::RightOrBottom,
        .Stops       = {0.0f, _orien == orientation::Vertical ? 1.0f - current_value() : current_value(), 1.0f}};
    thumb_element::context const thumbCtx {
        .Orientation      = barCtx.Orientation,
        .RelativePosition = current_value()};

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

auto scrollbar::mouse_hover(widget const& widget, point_i mp) -> bool
{
    if (_isDragging) { return is_mouse_over(); }

    _overThumb = false;
    _overBar   = false;

    if (!Visible) { return false; }

    auto const pos {global_to_parent(widget, mp)};

    if (_barRectCache.Thumb.contains(pos)) {
        _overThumb = true;
        return true;
    }

    // over bar
    if (_barRectCache.Bar.contains(pos)) {
        _overBar = true;
        return true;
    }

    return false;
}

auto scrollbar::mouse_drag(widget const& widget, point_i mp) -> bool
{
    if (_isDragging || is_mouse_over()) {
        calculate_value(global_to_content(widget, mp));
        _isDragging = true;
    }

    return _isDragging;
}

void scrollbar::mouse_down(widget const& widget, point_i mp)
{
    _isDragging = false;

    if (!is_mouse_over()) { return; }
    if (!_overThumb) {
        calculate_value(global_to_content(widget, mp));
    } else {
        _dragOffset = point_i {global_to_parent(widget, mp) - _barRectCache.Thumb.center()};
        _isDragging = true;
    }
}

void scrollbar::mouse_up(widget const& widget, point_i mp)
{
    _dragOffset = point_i::Zero;
    _isDragging = false;

    auto const pos {global_to_parent(widget, mp)};
    _overThumb = _barRectCache.Thumb.contains(pos);
    _overBar   = _barRectCache.Bar.contains(pos);
}

void scrollbar::mouse_leave()
{
    _dragOffset = point_i::Zero;
    _isDragging = false;

    _overThumb = false;
    _overBar   = false;
}

auto scrollbar::current_value() const -> f32
{
    return _tween.current_value();
}

auto scrollbar::target_value() const -> f32
{
    return _tween.target_value();
}

void scrollbar::start(f32 target)
{
    if (!Visible) {
        _tween.reset(target);
        return;
    }

    f32 const actTarget {std::clamp(target, 0.0f, 1.0f)};
    if (!_isDragging) {
        _tween.start(actTarget, _delay);
    } else {
        _tween.reset(actTarget);
    }
}

void scrollbar::reset(f32 target)
{
    _tween.reset(target);
    _delay = milliseconds::zero();
}

void scrollbar::calculate_value(point_f mp)
{
    rect_f const rect {_barRectCache.Bar};
    f32          frac {0.0f};

    switch (_orien) {
    case orientation::Horizontal: {
        f32 const tw {_barRectCache.Thumb.width()};
        f32 const width {rect.width() - tw};
        if (width > 0) { frac = (mp.X - static_cast<f32>(_dragOffset.X) - (tw / 2)) / width; }
    } break;
    case orientation::Vertical: {
        f32 const th {_barRectCache.Thumb.height()};
        f32 const height {rect.height() - th};
        if (height > 0) { frac = (mp.Y - static_cast<f32>(_dragOffset.Y) - (th / 2)) / height; }
    } break;
    }

    start(frac);

    _overThumb = true;
}

} // namespace ui
