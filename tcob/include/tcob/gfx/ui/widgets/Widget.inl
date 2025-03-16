// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Widget.hpp"

#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {

template <std::derived_from<widget_style> T>
inline void widget::apply_style(T& style)
{
    _transition.apply(style);
    _currentStyle = &style;
}

template <std::derived_from<style> T>
inline void widget::apply_sub_style(T& style, isize idx, string const& styleClass, widget_flags flags)
{
    widget_style_selectors const selectors {
        .Class      = styleClass,
        .Flags      = flags,
        .Attributes = attributes(),
    };
    auto* subStyle {static_cast<T*>(styles().get(selectors))};
    _subStyleTransitions[idx].try_start(subStyle, TransitionDuration);
    _subStyleTransitions[idx].apply(style);
}

inline auto widget::draw_background(auto&& style, widget_painter& painter, bool isCircle) -> rect_f
{
    apply_style(style);
    rect_f rect {Bounds()};
    painter.draw_background_and_border(style, rect, isCircle);
    return rect;
}

}
