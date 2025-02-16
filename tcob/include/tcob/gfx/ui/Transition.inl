// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Transition.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

template <typename T>
inline void transition<T>::start(T const* target, milliseconds duration)
{
    if (target == _targetStyle) { return; }

    _currentTime = milliseconds {0};
    _duration    = duration;

    _oldStyle     = _targetStyle;
    _targetStyle  = target;
    _currentStyle = _targetStyle;
}

template <typename T>
inline void transition<T>::reset(T const* target)
{
    _currentTime = milliseconds {0};
    _duration    = milliseconds {0};

    _targetStyle  = target;
    _oldStyle     = _targetStyle;
    _currentStyle = _targetStyle;
}

template <typename T>
void transition<T>::update(milliseconds deltaTime)
{
    _currentTime += deltaTime;
}

template <typename T>
auto transition<T>::is_active() const -> bool
{
    return _duration.count() > 0 && _currentTime <= _duration && _oldStyle && _targetStyle;
}

template <typename T>
inline auto transition<T>::current_style() const -> T const*
{
    return _currentStyle;
}

template <typename T>
template <typename S>
inline void transition<T>::update_style(S& style)
{
    if (_currentStyle) {
        assert(dynamic_cast<S const*>(_currentStyle));
        style = *static_cast<S const*>(_currentStyle);
    }

    if (is_active()) {
        f64 const frac {std::min(1.0, _currentTime.count() / _duration.count())};
        S::Transition(style, *static_cast<S const*>(_oldStyle), *static_cast<S const*>(_targetStyle), frac);
    }
    _currentStyle = &style;
}

}
