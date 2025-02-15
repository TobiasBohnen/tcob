// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/WidgetTweener.hpp"

namespace tcob::gfx::ui {

widget_tweener::widget_tweener(widget& parent)
    : _parent {parent}
{
}

void widget_tweener::start(f32 toValue, milliseconds delay)
{
    _targetValue = toValue;

    if (delay.count() == 0) {
        set_value(toValue);
    } else {
        _tween = make_unique_tween<linear_tween<f32>>(delay, _currentValue, toValue);
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
    _parent.force_redraw(_parent.name() + ": Tween value changed");
}

} // namespace ui
