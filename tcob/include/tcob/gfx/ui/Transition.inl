// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Transition.hpp"

#include <algorithm>
#include <cassert>

#include "tcob/gfx/ui/Style.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

template <std::derived_from<style> T>
inline void transition<T>::start(T const* target, milliseconds duration, milliseconds step)
{
    if (target == _targetStyle) { return; }

    _currentTime = milliseconds::zero();
    _duration    = duration;
    _accStep     = step;
    _step        = step;

    _sourceStyle = _targetStyle;
    _targetStyle = target;
}

template <std::derived_from<style> T>
inline void transition<T>::reset(T const* target)
{
    _currentTime = milliseconds::zero();
    _duration    = milliseconds::zero();
    _accStep     = milliseconds::zero();
    _step        = milliseconds::zero();

    _targetStyle = target;
    _sourceStyle = _targetStyle;
}

template <std::derived_from<style> T>
auto transition<T>::is_active() const -> bool
{
    return _duration.count() > 0 && _currentTime < _duration && _sourceStyle && _targetStyle;
}

template <std::derived_from<style> T>
auto transition<T>::update(milliseconds deltaTime) -> bool
{
    if (!is_active()) { return false; }

    _accStep -= deltaTime;
    if (_accStep.count() <= 0) {
        _currentTime = std::min(_currentTime + (_step - _accStep), _duration);
        _accStep     = _step;
        return true;
    }

    return false;
}

template <std::derived_from<style> T>
template <std::derived_from<style> S>
inline void transition<T>::apply(S& style)
{
    if (_targetStyle) {
        assert(dynamic_cast<S const*>(_targetStyle));
        style = *static_cast<S const*>(_targetStyle);
    }

    if (!is_active()) { return; }
    assert(dynamic_cast<S const*>(_sourceStyle));

    f64 const frac {_targetStyle->ease_value(_currentTime.count() / _duration.count())};
    S::Transition(style, *static_cast<S const*>(_sourceStyle), *static_cast<S const*>(_targetStyle), frac);
}

}
