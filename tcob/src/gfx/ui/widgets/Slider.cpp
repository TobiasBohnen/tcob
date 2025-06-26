// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Slider.hpp"

#include <algorithm>
#include <utility>

#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void slider::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    bar_element::Transition(target.Bar, left.Bar, right.Bar, step);
}

slider::slider(init const& wi)
    : widget {wi}
    , Min {{[this](i32 val) -> i32 { return std::min(val, *Max); }}}
    , Max {{[this](i32 val) -> i32 { return std::max(val, *Min); }}}
    , Value {{[this](i32 val) -> i32 { return std::clamp(val, *Min, *Max); }}}
{
    _tween.Changed.connect([this]() {
        request_redraw(this->name() + ": Tween value changed");
    });
    Min.Changed.connect([this](auto val) {
        Value = std::max(val, *Value);
        request_redraw(this->name() + ": Min changed");
    });
    Min(0);
    Max.Changed.connect([this](auto val) {
        Value = std::min(val, *Value);
        request_redraw(this->name() + ": Max changed");
    });
    Max(100);
    Step.Changed.connect([this](auto) { request_redraw(this->name() + ": Step changed"); });
    Step(1);
    Value.Changed.connect([this](auto val) { on_value_changed(val); });
    Value(*Min);

    Class("slider");
}

void slider::on_draw(widget_painter& painter)
{
    // TODO: draw background
    apply_style(_style);

    rect_f const rect {content_bounds()};

    scissor_guard const guard {painter, this};

    i32 const  numBlocks {10};
    auto const orien {get_orientation()};
    auto const pos {bar_element::position::CenterOrMiddle};

    // bar
    _barRectCache.Bar = painter.draw_bar(
        _style.Bar,
        rect,
        {.Orientation = orien,
         .Position    = pos,
         .BlockCount  = numBlocks,
         .Stops       = {0.0f, _tween.current_value(), 1.0f}});

    // thumb
    auto const  thumbFlags {!_overThumb          ? widget_flags {}
                                : flags().Active ? widget_flags {.Active = true}
                                                 : widget_flags {.Hover = true}};
    thumb_style thumbStyle {};
    apply_sub_style(thumbStyle, 0, _style.ThumbClass, thumbFlags);

    _barRectCache.Thumb = painter.draw_thumb(
        thumbStyle.Thumb,
        rect,
        {.Orientation      = orien,
         .RelativePosition = orien == orientation::Vertical ? 1.0f - _tween.current_value() : _tween.current_value()});

    if (orien == orientation::Vertical) {
        _barRectCache.Thumb.Size.Width -= thumbStyle.Thumb.Border.Size.calc(_barRectCache.Thumb.width());
    } else if (orien == orientation::Horizontal) {
        _barRectCache.Thumb.Size.Height -= thumbStyle.Thumb.Border.Size.calc(_barRectCache.Thumb.height());
    }
}

void slider::on_key_down(input::keyboard::event const& ev)
{
    auto const& controls {form().Controls};
    if (ev.Keyboard->is_key_down(controls->ActivateKey)) {
        if (ev.KeyCode == controls->NavLeftKey) {
            handle_dir_input(direction::Left);
            ev.Handled = true;
        } else if (ev.KeyCode == controls->NavRightKey) {
            handle_dir_input(direction::Right);
            ev.Handled = true;
        } else if (ev.KeyCode == controls->NavDownKey) {
            handle_dir_input(direction::Down);
            ev.Handled = true;
        } else if (ev.KeyCode == controls->NavUpKey) {
            handle_dir_input(direction::Up);
            ev.Handled = true;
        }
    }
}

void slider::on_mouse_leave()
{
    if (_overThumb) {
        _overThumb = false;
        request_redraw(this->name() + ": mouse left");
    }
}

void slider::on_mouse_hover(input::mouse::motion_event const& ev)
{
    bool const overThumb {_barRectCache.Thumb.contains(global_to_parent(*this, ev.Position))};
    if (overThumb != _overThumb) {
        _overThumb = overThumb;
        ev.Handled = true;
    }
}

void slider::on_mouse_drag(input::mouse::motion_event const& ev)
{
    if (_isDragging || _overThumb) {
        calculate_value(global_to_content(*this, ev.Position));
        _isDragging = true;
        ev.Handled  = true;
    }
}

void slider::on_mouse_button_up(input::mouse::button_event const& ev)
{
    _dragOffset = point_i::Zero;
    _isDragging = false;

    if (_overThumb && !hit_test(point_f {ev.Position})) {
        _overThumb = false;
        ev.Handled = true;
    }
}

void slider::on_mouse_button_down(input::mouse::button_event const& ev)
{
    _isDragging = false;

    if (ev.Button == controls().PrimaryMouseButton) {
        if (!_overThumb) {
            calculate_value(global_to_content(*this, ev.Position));
        } else {
            _dragOffset = point_i {global_to_parent(*this, ev.Position) - _barRectCache.Thumb.center()};
            _isDragging = true;
        }
        ev.Handled = true;
    }
}

void slider::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    if (!is_focused()) { return; }

    if (ev.Scroll.Y > 0) {
        Value += *Step;
    } else if (ev.Scroll.Y < 0) {
        Value -= *Step;
    } else if (ev.Scroll.X > 0) {
        Value += *Step;
    } else if (ev.Scroll.X < 0) {
        Value -= *Step;
    }

    ev.Handled = true;
}

void slider::on_controller_button_down(input::controller::button_event const& ev)
{
    auto const& controls {form().Controls};
    if (ev.Controller->is_button_pressed(controls->ActivateButton)) {
        if (ev.Button == controls->NavLeftButton) {
            handle_dir_input(direction::Left);
            ev.Handled = true;
        } else if (ev.Button == controls->NavRightButton) {
            handle_dir_input(direction::Right);
            ev.Handled = true;
        } else if (ev.Button == controls->NavDownButton) {
            handle_dir_input(direction::Down);
            ev.Handled = true;
        } else if (ev.Button == controls->NavUpButton) {
            handle_dir_input(direction::Up);
            ev.Handled = true;
        }
    }
}

void slider::on_update(milliseconds deltaTime)
{
    _tween.update(deltaTime);
}

void slider::on_value_changed(i32 newVal)
{
    f32 const newFrac {Max != Min ? static_cast<f32>(newVal - *Min) / static_cast<f32>(*Max - *Min) : 0.f};
    if (_isDragging) {
        _tween.reset(newFrac);
    } else {
        _tween.start(newFrac, _style.Bar.MotionDuration);
    }
}

void slider::calculate_value(point_f mp)
{
    rect_f const rect {_barRectCache.Bar};
    f32          frac {0.0f};

    switch (get_orientation()) {
    case orientation::Horizontal: {
        f32 const tw {_barRectCache.Thumb.width()};
        frac = (mp.X - static_cast<f32>(_dragOffset.X) - (tw / 2)) / (rect.width() - tw);
    } break;
    case orientation::Vertical: {
        f32 const th {_barRectCache.Thumb.height()};
        frac = 1.0f - ((mp.Y - static_cast<f32>(_dragOffset.Y) - (th / 2)) / (rect.height() - th));
    } break;
    }

    i32 const val {static_cast<i32>(*Min + ((*Max - *Min + 1) * frac))};
    Value = helper::round_to_multiple(val, *Step);

    if (!_overThumb) {
        _overThumb = true;
        request_redraw(this->name() + ": thumb move after value change");
    }
}

auto slider::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["min"]   = *Min;
    retValue["max"]   = *Max;
    retValue["value"] = *Value;
    retValue["step"]  = *Step;

    return retValue;
}

void slider::handle_dir_input(direction dir)
{
    switch (get_orientation()) {
    case orientation::Horizontal:
        switch (dir) {
        case direction::Left:  Value -= *Step; break;
        case direction::Right: Value += *Step; break;
        default:
            break;
        }
        break;
    case orientation::Vertical:
        switch (dir) {
        case direction::Down: Value -= *Step; break;
        case direction::Up:   Value += *Step; break;
        default:
            break;
        }
        break;
    }
}

////////////////////////////////////////////////////////////

range_slider::range_slider(init const& wi)
    : widget {wi}
    , Min {{[this](i32 val) -> i32 { return std::min(val, *Max); }}}
    , Max {{[this](i32 val) -> i32 { return std::max(val, *Min); }}}
    , MinRange {{[this](i32 val) -> i32 { return std::min(val, *MaxRange); }}}
    , MaxRange {{[this](i32 val) -> i32 { return std::max(val, *MinRange); }}}
    , Values {{[this](std::pair<i32, i32> val) -> std::pair<i32, i32> {
        i32 first {std::clamp(val.first, *Min, *Max)};
        i32 second {std::clamp(val.second, *Min, *Max)};
        if (first > second) { std::swap(first, second); }

        i32 range {second - first};
        if (range < *MinRange) { return *Values; }
        if (range > *MaxRange) { return *Values; }

        return {first, second};
    }}}
{
    _min.Tween.Changed.connect([this]() {
        request_redraw(this->name() + ": Tween value changed");
    });
    _max.Tween.Changed.connect([this]() {
        request_redraw(this->name() + ": Tween value changed");
    });
    Min.Changed.connect([this](auto val) {
        Values = {std::max(val, Values->first), std::max(val, Values->second)};
        request_redraw(this->name() + ": Min changed");
    });
    Min(0);
    Max.Changed.connect([this](auto val) {
        Values = {std::min(val, Values->first), std::min(val, Values->second)};
        request_redraw(this->name() + ": Max changed");
    });
    Max(100);

    MinRange.Changed.connect([this](auto) {
        auto [first, second] {*Values};
        i32 const range {second - first};

        if (range < *MinRange) {
            if (second + (*MinRange - range) <= *Max) {
                second += (*MinRange - range);
            } else {
                first = second - *MinRange;
            }
        }

        Values = {first, second};
        request_redraw(this->name() + ": MinRange changed");
    });
    MinRange(0);
    MaxRange.Changed.connect([this](auto) {
        auto [first, second] {*Values};
        i32 const range {second - first};

        if (range > *MaxRange) {
            second = std::min(second, first + *MaxRange);
        }

        Values = {first, second};
        request_redraw(this->name() + ": MaxRange changed");
    });
    MaxRange(100);

    Step.Changed.connect([this](auto) { request_redraw(this->name() + ": Step changed"); });
    Step(1);
    Values.Changed.connect([this](auto val) { on_value_changed(val); });
    Values({*Min, *Min});

    Class("range_slider");
}

void range_slider::on_draw(widget_painter& painter)
{
    // TODO: draw background
    apply_style(_style);

    rect_f const rect {content_bounds()};

    scissor_guard const guard {painter, this};

    i32 const  numBlocks {10};
    auto const orien {get_orientation()};
    auto const pos {bar_element::position::CenterOrMiddle};

    // bar
    _barRectCache = painter.draw_bar(
        _style.Bar,
        rect,
        {.Orientation = orien,
         .Position    = pos,
         .BlockCount  = numBlocks,
         .Stops       = {0.0f, _min.Tween.current_value(), _max.Tween.current_value(), 1.0f}});

    // thumb
    thumb_style thumbStyle {};

    auto const drawThumb {[&](thumb& thumb, i32 idx) {
        auto const thumbFlags {!thumb.Over          ? widget_flags {}
                                   : flags().Active ? widget_flags {.Active = true}
                                                    : widget_flags {.Hover = true}};
        apply_sub_style(thumbStyle, idx, _style.ThumbClass, thumbFlags);

        thumb.Rect = painter.draw_thumb(
            thumbStyle.Thumb,
            rect,
            {.Orientation      = orien,
             .RelativePosition = orien == orientation::Vertical ? 1.0f - thumb.Tween.current_value() : thumb.Tween.current_value()});
    }};

    if (_min.Over) {
        drawThumb(_max, 1);
        drawThumb(_min, 0);
    } else {
        drawThumb(_min, 0);
        drawThumb(_max, 1);
    }

    if (orien == orientation::Vertical) {
        _min.Rect.Size.Width -= thumbStyle.Thumb.Border.Size.calc(_min.Rect.width());
        _max.Rect.Size.Width -= thumbStyle.Thumb.Border.Size.calc(_max.Rect.width());
    } else if (orien == orientation::Horizontal) {
        _min.Rect.Size.Height -= thumbStyle.Thumb.Border.Size.calc(_min.Rect.height());
        _max.Rect.Size.Height -= thumbStyle.Thumb.Border.Size.calc(_max.Rect.height());
    }
}

void range_slider::on_mouse_leave()
{
    if (_min.Over || _max.Over) {
        _min.Over = false;
        _max.Over = false;
        request_redraw(this->name() + ": mouse left");
    }
}

void range_slider::on_mouse_hover(input::mouse::motion_event const& ev)
{
    auto const mp {global_to_parent(*this, ev.Position)};

    auto const hover {[&](thumb& thumb) {
        bool const overThumb {thumb.Rect.contains(mp)};
        if (overThumb != thumb.Over) {
            thumb.Over = overThumb;
            return true;
        }
        return false;
    }};

    ev.Handled = hover(_min) || hover(_max);
}

void range_slider::on_mouse_drag(input::mouse::motion_event const& ev)
{
    auto const mp {global_to_content(*this, ev.Position)};

    auto const drag {[&](thumb& thumb) {
        if (thumb.IsDragging || thumb.Over) {
            calculate_value(thumb, mp);
            thumb.IsDragging = true;
            return true;
        }
        return false;
    }};

    ev.Handled = drag(_min) || drag(_max);
}

void range_slider::on_mouse_button_up(input::mouse::button_event const& ev)
{
    _dragOffset = point_i::Zero;

    _min.IsDragging = false;
    _max.IsDragging = false;

    auto const buttonUp {[&](thumb& thumb) {
        if (thumb.Over && !hit_test(point_f {ev.Position})) {
            thumb.Over = false;
            return true;
        }
        return false;
    }};

    ev.Handled = buttonUp(_min) || buttonUp(_max);
}

void range_slider::on_mouse_button_down(input::mouse::button_event const& ev)
{
    auto const mp {global_to_parent(*this, ev.Position)};

    _min.IsDragging = false;
    _max.IsDragging = false;

    if (ev.Button == controls().PrimaryMouseButton) {
        if (_min.Over) {
            _dragOffset     = point_i {mp - _min.Rect.center()};
            _min.IsDragging = true;
        } else if (_max.Over) {
            _dragOffset     = point_i {mp - _max.Rect.center()};
            _max.IsDragging = true;
        } else {
            // calculate_value(global_to_content(*this, ev.Position));
        }

        ev.Handled = true;
    }
}

void range_slider::on_update(milliseconds deltaTime)
{
    _min.Tween.update(deltaTime);
    _max.Tween.update(deltaTime);
}

void range_slider::on_value_changed(std::pair<i32, i32> newVal)
{
    auto const value {[&](thumb& thumb, f32 val) {
        f32 const newFrac0 {Max != Min ? static_cast<f32>(val - *Min) / static_cast<f32>(*Max - *Min) : 0.f};
        if (thumb.IsDragging) {
            thumb.Tween.reset(newFrac0);
        } else {
            thumb.Tween.start(newFrac0, _style.Bar.MotionDuration);
        }
    }};

    value(_min, newVal.first);
    value(_max, newVal.second);
}

auto range_slider::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["min"]       = *Min;
    retValue["max"]       = *Max;
    retValue["min_range"] = *MinRange;
    retValue["max_range"] = *MaxRange;
    retValue["min_value"] = Values->first;
    retValue["max_value"] = Values->second;
    retValue["step"]      = *Step;

    return retValue;
}

void range_slider::calculate_value(thumb& thumb, point_f mp)
{
    rect_f const rect {_barRectCache};
    f32          frac {0.0f};

    switch (get_orientation()) {
    case orientation::Horizontal: {
        f32 const tw {thumb.Rect.width()};
        frac = (mp.X - static_cast<f32>(_dragOffset.X) - (tw / 2)) / (rect.width() - tw);
    } break;
    case orientation::Vertical: {
        f32 const th {thumb.Rect.height()};
        frac = 1.0f - ((mp.Y - static_cast<f32>(_dragOffset.Y) - (th / 2)) / (rect.height() - th));
    } break;
    }

    i32 const val {static_cast<i32>(*Min + ((*Max - *Min + 1) * frac))};
    if (&thumb == &_min) {
        Values = {helper::round_to_multiple(val, *Step), Values->second};
    } else {
        Values = {Values->first, helper::round_to_multiple(val, *Step)};
    }
    if (!thumb.Over) {
        thumb.Over = true;
        request_redraw(this->name() + ": thumb move after value change");
    }
}

} // namespace ui
