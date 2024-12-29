// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Scrollbar.hpp"

namespace tcob::gfx::ui {

template <typename Parent>
inline scrollbar<Parent>::scrollbar(Parent& parent, orientation orien)
    : _orien {orien}
    , _parent {parent}
    , _tween {parent}
{
}

template <typename Parent>
inline void scrollbar<Parent>::update(milliseconds deltaTime)
{
    _tween.update(deltaTime);
}

template <typename Parent>
inline void scrollbar<Parent>::paint(widget_painter& painter, element::scrollbar const& style, rect_f& rect, bool isActive)
{
    Visible = requires_scroll(rect);
    if (Visible) {
        i32 const numBlocks {10};
        f32 const min {get_min_value()};
        f32 const max {get_max_value()};
        f32 const frac {(current_value() - min) / ((max - min))};

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
        _paintResult = painter.draw_scrollbar(style, get_thumb_style(thumbFlags)->Thumb, scrRect, barCtx);

        if (_orien == orientation::Vertical) {
            rect.Size.Width -= _paintResult.Bar.width() + style.Bar.Border.Size.calc(_paintResult.Bar.width());
        } else if (_orien == orientation::Horizontal) {
            rect.Size.Height -= _paintResult.Bar.height() + style.Bar.Border.Size.calc(_paintResult.Bar.height());
        }
    }
}

template <typename Parent>
inline auto scrollbar<Parent>::is_mouse_over() const -> bool
{
    return Visible && (_overBar || _overThumb);
}

template <typename Parent>
inline auto scrollbar<Parent>::inject_mouse_hover(point_i mp) -> bool
{
    if (_isDragging) { return false; }

    _overThumb = false;
    _overBar   = false;

    if (!Visible) { return false; }

    if (_paintResult.Thumb.contains(_parent.global_to_parent_local(mp))) {
        _overThumb = true;
        return true;
    }

    // over bar
    if (_paintResult.Bar.contains(_parent.global_to_parent_local(mp))) {
        _overBar = true;
        return true;
    }

    return false;
}

template <typename Parent>
inline auto scrollbar<Parent>::inject_mouse_drag(point_i mp) -> bool
{
    if (_isDragging || is_mouse_over()) {
        calculate_value(_parent.global_to_local(mp));
        _isDragging = true;
        return true;
    }

    return false;
}

template <typename Parent>
inline void scrollbar<Parent>::inject_mouse_up(point_i mp)
{
    _dragOffset = point_i::Zero;
    _isDragging = false;

    _overThumb = _paintResult.Thumb.contains(_parent.global_to_parent_local(mp));
    _overBar   = _paintResult.Bar.contains(_parent.global_to_parent_local(mp));
}

template <typename Parent>
inline void scrollbar<Parent>::inject_mouse_down(point_i mp)
{
    _isDragging = false;

    if (is_mouse_over()) {
        if (!_overThumb) {
            calculate_value(_parent.global_to_local(mp));
        } else {
            _dragOffset = point_i {_parent.global_to_parent_local(mp) - _paintResult.Thumb.center()};
            _isDragging = true;
        }
    }
}

template <typename Parent>
inline auto scrollbar<Parent>::requires_scroll(rect_f const& rect) const -> bool
{
    return _parent.requires_scroll(_orien, rect);
}

template <typename Parent>
inline auto scrollbar<Parent>::current_value() const -> f32
{
    return _tween.current_value();
}

template <typename Parent>
inline auto scrollbar<Parent>::target_value() const -> f32
{
    return _tween.target_value();
}

template <typename Parent>
inline void scrollbar<Parent>::set_target_value(f32 val, milliseconds delay)
{
    if (!Visible) {
        _tween.reset(val);
        return;
    }

    f32 const newVal {std::clamp(val, get_min_value(), get_max_value())};
    if (!_isDragging) {
        _tween.start(newVal, delay);
    } else {
        _tween.reset(newVal);
    }
}

template <typename Parent>
inline auto scrollbar<Parent>::get_min_value() const -> f32
{
    return _parent.get_scroll_min_value(_orien);
}

template <typename Parent>
inline auto scrollbar<Parent>::get_max_value() const -> f32
{
    return _parent.get_scroll_max_value(_orien);
}

template <typename Parent>
inline void scrollbar<Parent>::calculate_value(point_f mp)
{
    if (auto* const style {_parent.get_scroll_style(_orien)}) {
        rect_f const rect {_paintResult.Bar};
        f32          frac {0.0f};
        milliseconds delay {};

        switch (_orien) {
        case orientation::Horizontal: {
            f32 const tw {_paintResult.Thumb.width()};
            frac  = (mp.X - _dragOffset.X - (tw / 2)) / (rect.width() - tw);
            delay = style->Bar.Delay;
        } break;
        case orientation::Vertical: {
            f32 const th {_paintResult.Thumb.height()};
            frac  = (mp.Y - _dragOffset.Y - (th / 2)) / (rect.height() - th);
            delay = style->Bar.Delay;
        } break;
        }

        f32 const min {get_min_value()};
        f32 const max {get_max_value()};
        set_target_value(min + ((max - min) * frac), delay);

        _overThumb = true;
    }
}

template <typename Parent>
inline auto scrollbar<Parent>::get_thumb_style(widget_flags flags) -> thumb_style*
{
    if (auto* const style {_parent.get_scroll_style(_orien)}) {
        return static_cast<thumb_style*>(_parent.parent_form()->Styles->get(style->ThumbClass, flags, {}));
    }

    return nullptr;
}

} // namespace ui
