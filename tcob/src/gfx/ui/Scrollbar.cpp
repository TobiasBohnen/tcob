// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Scrollbar.hpp"

#include "tcob/gfx/ui/Form.hpp"
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

void scrollbar::paint(widget_painter& painter, element::scrollbar const& style, rect_f& rect, bool isActive)
{
    if (!Visible) { return; }

    i32 const numBlocks {10};
    f32 const frac {(current_value() - Min) / ((Max - Min))};

    element::bar::context const barCtx {
        .Orientation = _orien,
        .Inverted    = _orien == orientation::Vertical,
        .Position    = element::bar::position::RightOrBottom,
        .BlockCount  = numBlocks,
        .Fraction    = frac};

    rect_f const scrRect {rect};
    auto const   thumbFlags {!_overThumb    ? widget_flags {}
                                 : isActive ? widget_flags {.Active = true}
                                            : widget_flags {.Hover = true}};
    _barRectCache = painter.draw_scrollbar(style, get_thumb_style(thumbFlags)->Thumb, scrRect, barCtx);

    if (_orien == orientation::Vertical) {
        rect.Size.Width -= _barRectCache.Bar.width() + style.Bar.Border.Size.calc(_barRectCache.Bar.width());
    } else if (_orien == orientation::Horizontal) {
        rect.Size.Height -= _barRectCache.Bar.height() + style.Bar.Border.Size.calc(_barRectCache.Bar.height());
    }
}

auto scrollbar::is_mouse_over() const -> bool
{
    return Visible && (_overBar || _overThumb);
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

    f32 const actTarget {std::clamp(target, Min, Max)};
    if (!_isDragging) {
        _tween.start(actTarget, delay);
    } else {
        _tween.reset(actTarget);
    }
}

void scrollbar::calculate_value(point_f mp)
{
    if (!Style) { return; }

    rect_f const rect {_barRectCache.Bar};
    f32          frac {0.0f};
    milliseconds delay {};

    switch (_orien) {
    case orientation::Horizontal: {
        f32 const tw {_barRectCache.Thumb.width()};
        frac  = (mp.X - _dragOffset.X - (tw / 2)) / (rect.width() - tw);
        delay = Style->Bar.Delay;
    } break;
    case orientation::Vertical: {
        f32 const th {_barRectCache.Thumb.height()};
        frac  = (mp.Y - _dragOffset.Y - (th / 2)) / (rect.height() - th);
        delay = Style->Bar.Delay;
    } break;
    }

    start_scroll(Min + ((Max - Min) * frac), delay);

    _overThumb = true;
}

auto scrollbar::get_thumb_style(widget_flags flags) -> thumb_style*
{
    assert(Style);
    widget_style_selectors const selectors {
        .Class      = Style->ThumbClass,
        .Flags      = flags,
        .Attributes = {},
    };
    return dynamic_cast<thumb_style*>(_parent.parent_form()->Styles->get(selectors));
}

} // namespace ui
