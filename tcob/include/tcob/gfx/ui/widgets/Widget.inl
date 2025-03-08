// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Widget.hpp"

#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::gfx::ui {

template <std::derived_from<widget_style> T>
inline void widget::update_style(T& style)
{
    _transition.apply(style);
    _currentStyle = &style;
}

template <std::derived_from<style> T>
inline void widget::update_sub_style(T& style, isize idx, string const& styleClass, widget_flags flags)
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

}
