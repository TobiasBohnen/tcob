// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Transition.hpp"

#include <algorithm>
#include <cassert>

namespace tcob::ui {
////////////////////////////////////////////////////////////

template <typename T>
inline void transition<T>::try_start(T const* target, milliseconds duration)
{
    if (target == _targetStyle) { return; }

    _currentTime = milliseconds::zero();
    _duration    = duration;

    _sourceStyle = _targetStyle;
    _targetStyle = target;
}

template <typename T>
inline void transition<T>::reset(T const* target)
{
    _currentTime = milliseconds::zero();
    _duration    = milliseconds::zero();

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
inline void transition<T>::apply(S& style)
{
    if (_targetStyle) {
#if defined(TCOB_DEBUG)
        auto dp {dynamic_cast<S const*>(_targetStyle)};
        assert(dp);
        style = *dp;
#else
        style = *static_cast<S const*>(_targetStyle);
#endif
    }

    if (!is_active()) { return; }

    f64 const frac {_targetStyle->ease_value(_currentTime.count() / _duration.count())};
    S::Transition(style, *static_cast<S const*>(_sourceStyle), *static_cast<S const*>(_targetStyle), frac);
}

}
