// Copyright (c) 2023 Tobias Bohnen
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
    _toval = toValue;

    if (delay.count() == 0) {
        _val = toValue;
        _parent.force_redraw(_parent.get_name() + ": Tween start");
    } else {
        using namespace tcob::tweening;
        _tween = make_unique_tween<linear_tween<f32>>(delay, _val, toValue);
        _tween->Value.Changed.connect([&](auto val) {
            _val = val;
            _parent.force_redraw(_parent.get_name() + ": Tween value changed");
        });
        _tween->start();
    }
}

void widget_tweener::update(milliseconds deltaTime)
{
    if (_tween) {
        _tween->update(deltaTime);
    }
}

auto widget_tweener::get_current_value() const -> f32
{
    return _val;
}

auto widget_tweener::get_target_value() const -> f32
{
    return _toval;
}

void widget_tweener::reset(f32 value)
{
    _val   = value;
    _toval = value;
    if (_tween) {
        _tween->stop();
    }
}

} // namespace ui
