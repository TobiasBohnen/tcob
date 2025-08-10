// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Transition.hpp"

#include <algorithm>
#include <cassert>

#include "tcob/core/easing/Easing.hpp"

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

    f64 frac {_currentTime.count() / _duration.count()};
    switch (_targetStyle->EasingFunc) {
    case easing_func::Linear:       break;
    case easing_func::SmoothStep:   frac = easing::smoothstep<f32> {.Start = 0, .End = 1}(frac); break;
    case easing_func::SmootherStep: frac = easing::smootherstep<f32> {.Start = 0, .End = 1}(frac); break;
    case easing_func::QuadIn:       frac = easing::power<f32> {.Start = 0, .End = 1, .Exponent = 2}(frac); break;
    case easing_func::QuadOut:      frac = easing::inverse_power<f32> {.Start = 0, .End = 1, .Exponent = 2}(frac); break;
    case easing_func::QuadInOut:    frac = easing::inout_power<f32> {.Start = 0, .End = 1., .Exponent = 2}(frac); break;
    case easing_func::CubicIn:      frac = easing::power<f32> {.Start = 0, .End = 1, .Exponent = 3}(frac); break;
    case easing_func::CubicOut:     frac = easing::inverse_power<f32> {.Start = 0, .End = 1, .Exponent = 3}(frac); break;
    case easing_func::CubicInOut:   frac = easing::inout_power<f32> {.Start = 0, .End = 1., .Exponent = 3}(frac); break;
    case easing_func::QuartIn:      frac = easing::power<f32> {.Start = 0, .End = 1, .Exponent = 4}(frac); break;
    case easing_func::QuartOut:     frac = easing::inverse_power<f32> {.Start = 0, .End = 1, .Exponent = 4}(frac); break;
    case easing_func::QuartInOut:   frac = easing::inout_power<f32> {.Start = 0, .End = 1., .Exponent = 4}(frac); break;
    case easing_func::QuintIn:      frac = easing::power<f32> {.Start = 0, .End = 1, .Exponent = 5}(frac); break;
    case easing_func::QuintOut:     frac = easing::inverse_power<f32> {.Start = 0, .End = 1, .Exponent = 5}(frac); break;
    case easing_func::QuintInOut:   frac = easing::inout_power<f32> {.Start = 0, .End = 1., .Exponent = 5}(frac); break;
    case easing_func::ExpoIn:       frac = easing::exponential<f32> {.Start = 0, .End = 1.}(frac); break;
    case easing_func::ExpoOut:      frac = easing::inverse_exponential<f32> {.Start = 0, .End = 1.}(frac); break;
    case easing_func::ExpoInOut:    frac = easing::inout_exponential<f32> {.Start = 0, .End = 1.}(frac); break;
    }

    S::Transition(style, *static_cast<S const*>(_sourceStyle), *static_cast<S const*>(_targetStyle), frac);
}

}
