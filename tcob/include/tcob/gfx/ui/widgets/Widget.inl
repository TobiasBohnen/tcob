// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Widget.hpp"

namespace tcob::gfx::ui {

template <std::derived_from<widget_style> T>
inline void widget::update_style(T& style)
{
    if (_transition.CurrentStyle) {
        assert(dynamic_cast<T*>(_transition.CurrentStyle));
        style = *static_cast<T*>(_transition.CurrentStyle);
    }

    if (_transition.is_active()) {
        T::Transition(style, *static_cast<T*>(_transition.OldStyle), *static_cast<T*>(_transition.TargetStyle), _transition.Tween->Value);
    }
    _transition.CurrentStyle = &style;
}

template <std::derived_from<style> T>
inline auto widget::get_sub_style(string const& styleClass, widget_flags flags) const -> T*
{
    widget_style_selectors const selectors {
        .Class      = styleClass,
        .Flags      = flags,
        .Attributes = attributes(),
    };
    return static_cast<T*>(styles().get(selectors));
}

}
