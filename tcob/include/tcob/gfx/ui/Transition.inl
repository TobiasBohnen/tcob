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

    _sourceStyle = _targetStyle;
    _targetStyle = target;
}

template <typename T>
inline void transition<T>::reset(T const* target)
{
    _currentTime = milliseconds {0};
    _duration    = milliseconds {0};

    _targetStyle = target;
    _sourceStyle = _targetStyle;
}

template <typename T>
void transition<T>::update(milliseconds deltaTime)
{
    _currentTime = std::min(_currentTime + deltaTime, _duration);
}

template <typename T>
auto transition<T>::is_active() const -> bool
{
    return _duration.count() > 0 && _currentTime < _duration && _sourceStyle && _targetStyle;
}

template <typename T>
template <typename S>
inline void transition<T>::update_style(S& style)
{
    if (_targetStyle) {
        assert(dynamic_cast<S const*>(_targetStyle));
        style = *static_cast<S const*>(_targetStyle);
    }

    if (is_active()) {
        f64 const frac {_currentTime.count() / _duration.count()};
        S::Transition(style, *static_cast<S const*>(_sourceStyle), *static_cast<S const*>(_targetStyle), frac);
    }
}
}
