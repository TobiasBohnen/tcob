// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <memory>

#include "tcob/gfx/ui/WidgetTweener.hpp"

#include "tcob/core/Common.hpp"
#include "tcob/gfx/animation/Animation.hpp"
#include "tcob/gfx/animation/Tween.hpp"

namespace tcob::ui {

widget_tweener::widget_tweener() = default;

void widget_tweener::start(f32 toValue, milliseconds delay)
{
    _targetValue = toValue;

    if (delay.count() == 0) {
        set_value(toValue);
    } else {
        _tween = gfx::make_unique_tween<gfx::linear_tween<f32>>(delay, _currentValue, toValue);
        _tween->Value.Changed.connect([&](auto value) { set_value(value); });
        _tween->start();
    }
}

void widget_tweener::update(milliseconds deltaTime)
{
    if (_tween) {
        _tween->update(deltaTime);
    }
}

auto widget_tweener::current_value() const -> f32
{
    return _currentValue;
}

auto widget_tweener::target_value() const -> f32
{
    return _targetValue;
}

void widget_tweener::reset(f32 value)
{
    set_value(value);
    _targetValue = value;
    if (_tween) { _tween->stop(); }
}

void widget_tweener::set_value(f32 value)
{
    _currentValue = value;
    Changed();
}

////////////////////////////////////////////////////////////

animation_tweener::animation_tweener() = default;

void animation_tweener::start(gfx::frame_animation const& ani, playback_mode mode)
{
    if (ani.Frames.empty()) { stop(); }

    _tween = std::make_unique<gfx::frame_animation_tween>(ani.duration(), ani);
    _tween->Value.Changed.connect([this](auto const& str) {
        Changed(str);
    });

    _tween->start(mode);
}

void animation_tweener::stop()
{
    if (_tween) {
        _tween->stop();
    }
}

void animation_tweener::update(milliseconds deltaTime)
{
    if (_tween) {
        _tween->update(deltaTime);
    }
}

} // namespace ui
