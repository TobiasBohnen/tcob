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

icon_tweener::icon_tweener() = default;

void icon_tweener::start(playback_mode mode)
{
    if (_tween) {
        _tween->start(mode);
    }
}

void icon_tweener::stop()
{
    if (_tween) {
        _tween->stop();
    }
}

void icon_tweener::update(milliseconds deltaTime)
{
    if (_tween) {
        _tween->update(deltaTime);
    }
}

void icon_tweener::animation(gfx::frame_animation const& ani)
{
    _ani = ani;
    if (_ani.Frames.empty()) {
        _tween = nullptr;
        return;
    }

    _tween = std::make_unique<gfx::frame_animation_tween>(_ani.duration(), _ani);
    _tween->Value.Changed.connect([this](auto const& str) {
        Changed(str);
    });
}

} // namespace ui
