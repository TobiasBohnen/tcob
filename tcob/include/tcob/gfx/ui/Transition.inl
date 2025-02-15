// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Transition.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

template <typename T>
inline void transition_context<T>::start(T* target, milliseconds duration)
{
    _currentTime = milliseconds {0};
    _duration    = duration;

    _oldStyle     = _targetStyle;
    _targetStyle  = target;
    _currentStyle = _targetStyle;
}

template <typename T>
inline void transition_context<T>::reset(T* target)
{
    _currentTime = milliseconds {0};
    _duration    = milliseconds {0};

    _targetStyle  = target;
    _oldStyle     = _targetStyle;
    _currentStyle = _targetStyle;
}

template <typename T>
auto transition_context<T>::update(milliseconds deltaTime) -> bool
{
    bool const retValue {is_active()};
    _currentTime += deltaTime;
    return retValue;
}

template <typename T>
auto transition_context<T>::is_active() const -> bool
{
    return _duration.count() > 0 && _currentTime <= _duration && _oldStyle && _targetStyle;
}

template <typename T>
inline auto transition_context<T>::current_style() const -> T*
{
    return _currentStyle;
}

template <typename T>
template <typename S>
inline void transition_context<T>::update_style(S& style)
{
    if (_currentStyle) {
        assert(dynamic_cast<S*>(_currentStyle));
        style = *static_cast<S*>(_currentStyle);
    }

    if (is_active()) {
        f64 const frac {std::min(1.0, _currentTime.count() / _duration.count())};
        S::Transition(style, *static_cast<S*>(_oldStyle), *static_cast<S*>(_targetStyle), frac);
    }
    _currentStyle = &style;
}

}
