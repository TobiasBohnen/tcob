// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Slider.hpp"

#include <algorithm>
#include <cstdlib>

#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {

void slider::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    element::bar::Transition(target.Bar, left.Bar, right.Bar, step);
}

slider::slider(init const& wi)
    : widget {wi}
    , Min {{[this](i32 val) -> i32 { return std::min(val, Max()); }}}
    , Max {{[this](i32 val) -> i32 { return std::max(val, Min()); }}}
    , Value {{[this](i32 val) -> i32 {
        if (IncrementalChange && std::abs(Value() - val) > Step) { return Value(); }
        return std::clamp(val, Min(), Max());
    }}}
    , _tween {*this}
{
    Min.Changed.connect([this](auto val) {
        Value = std::max(val, Value());
        on_value_changed(Value());
        force_redraw(this->name() + ": Min changed");
    });
    Min(0);
    Max.Changed.connect([this](auto val) {
        Value = std::min(val, Value());
        on_value_changed(Value());
        force_redraw(this->name() + ": Max changed");
    });
    Max(100);
    Step.Changed.connect([this](auto) { force_redraw(this->name() + ": Step changed"); });
    Step(1);
    Value.Changed.connect([this](auto val) { on_value_changed(val); });
    Value(Min());

    Class("slider");
}

void slider::on_paint(widget_painter& painter)
{
    // TODO: draw background
    update_style(_style);

    rect_f const rect {content_bounds()};

    scissor_guard const guard {painter, this};

    i32 const  numBlocks {10};
    auto const orien {current_orientation()};
    auto const pos {element::bar::position::CenterOrMiddle};

    // bar
    _barRectCache.Bar = painter.draw_bar(
        _style.Bar,
        rect,
        {.Orientation = orien,
         .Inverted    = false,
         .Position    = pos,
         .BlockCount  = numBlocks,
         .Fraction    = _tween.current_value()});

    // thumb
    auto const  thumbFlags {!_overThumb          ? widget_flags {}
                                : flags().Active ? widget_flags {.Active = true}
                                                 : widget_flags {.Hover = true}};
    thumb_style thumbStyle {};
    update_sub_style(thumbStyle, 0, _style.ThumbClass, thumbFlags);

    _barRectCache.Thumb = painter.draw_thumb(
        thumbStyle.Thumb,
        rect,
        {.Orientation = orien, .Inverted = false, .Fraction = _tween.current_value()});

    if (orien == orientation::Vertical) {
        _barRectCache.Thumb.Size.Width -= thumbStyle.Thumb.Border.Size.calc(_barRectCache.Thumb.width());
    } else if (orien == orientation::Horizontal) {
        _barRectCache.Thumb.Size.Height -= thumbStyle.Thumb.Border.Size.calc(_barRectCache.Thumb.height());
    }
}

void slider::on_key_down(input::keyboard::event const& ev)
{
    auto const& controls {parent_form()->Controls};
    if (locate_service<input::system>().keyboard().is_key_down(controls->ActivateKey)) {
        if (ev.KeyCode == controls->NavLeftKey) {
            handle_dir_input(direction::Left);
        } else if (ev.KeyCode == controls->NavRightKey) {
            handle_dir_input(direction::Right);
        } else if (ev.KeyCode == controls->NavDownKey) {
            handle_dir_input(direction::Down);
        } else if (ev.KeyCode == controls->NavUpKey) {
            handle_dir_input(direction::Up);
        }
        ev.Handled = true;
    }
}

void slider::on_mouse_leave()
{
    if (_overThumb) {
        _overThumb = false;
        force_redraw(this->name() + ": mouse left");
    }
}

void slider::on_mouse_hover(input::mouse::motion_event const& ev)
{
    bool const overThumb {_barRectCache.Thumb.contains(global_to_local(ev.Position))};
    if (overThumb != _overThumb) {
        _overThumb = overThumb;
        force_redraw(this->name() + ": thumb hover change");
        ev.Handled = true;
    }
}

void slider::on_mouse_drag(input::mouse::motion_event const& ev)
{
    if (_isDragging || _overThumb) {
        calculate_value(global_to_content(ev.Position));
        _isDragging = true;
        ev.Handled  = true;
    }
}

void slider::on_mouse_up(input::mouse::button_event const& ev)
{
    _dragOffset = point_i::Zero;
    _isDragging = false;

    if (_overThumb && !hit_test(point_f {ev.Position})) {
        _overThumb = false;
        force_redraw(this->name() + ": thumb left");
        ev.Handled = true;
    }
}

void slider::on_mouse_down(input::mouse::button_event const& ev)
{
    _isDragging = false;

    if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
        if (!_overThumb) {
            calculate_value(global_to_content(ev.Position));
        } else {
            _dragOffset = point_i {global_to_local(ev.Position) - _barRectCache.Thumb.center()};
            _isDragging = true;
        }
        ev.Handled = true;
    }
}

void slider::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    if (!is_focused()) { return; }

    if (ev.Scroll.Y > 0) {
        Value += Step();
    } else if (ev.Scroll.Y < 0) {
        Value -= Step();
    } else if (ev.Scroll.X > 0) {
        Value += Step();
    } else if (ev.Scroll.X < 0) {
        Value -= Step();
    }

    ev.Handled = true;
}

void slider::on_controller_button_down(input::controller::button_event const& ev)
{
    auto const& controls {parent_form()->Controls};
    if (ev.Controller->is_button_pressed(controls->ActivateButton)) {
        if (ev.Button == controls->NavLeftButton) {
            handle_dir_input(direction::Left);
        } else if (ev.Button == controls->NavRightButton) {
            handle_dir_input(direction::Right);
        } else if (ev.Button == controls->NavDownButton) {
            handle_dir_input(direction::Down);
        } else if (ev.Button == controls->NavUpButton) {
            handle_dir_input(direction::Up);
        }
        ev.Handled = true;
    }
}

void slider::on_update(milliseconds deltaTime)
{
    _tween.update(deltaTime);
}

void slider::on_value_changed(i32 newVal)
{
    f32 const newFrac {Max != Min ? static_cast<f32>(newVal - Min()) / (Max() - Min()) : 0.f};
    if (_isDragging) {
        _tween.reset(newFrac);
    } else {
        _tween.start(newFrac, _style.Bar.Delay);
    }
}

void slider::calculate_value(point_f mp)
{
    rect_f const rect {_barRectCache.Bar};
    f32          frac {0.0f};

    switch (current_orientation()) {
    case orientation::Horizontal: {
        f32 const tw {_barRectCache.Thumb.width()};
        frac = (mp.X - _dragOffset.X - (tw / 2)) / (rect.width() - tw);
    } break;
    case orientation::Vertical: {
        f32 const th {_barRectCache.Thumb.height()};
        frac = 1.0f - ((mp.Y - _dragOffset.Y - (th / 2)) / (rect.height() - th));
    } break;
    }

    i32 const val {static_cast<i32>(Min() + ((Max() - Min() + 1) * frac))};
    Value = helper::round_to_multiple(val, Step());

    if (!_overThumb) {
        _overThumb = true;
        force_redraw(this->name() + ": thumb move after value change");
    }
}

auto slider::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["min"]   = Min();
    retValue["max"]   = Max();
    retValue["value"] = Value();
    retValue["step"]  = Step();

    return retValue;
}

void slider::handle_dir_input(direction dir)
{
    switch (current_orientation()) {
    case orientation::Horizontal:
        switch (dir) {
        case direction::Left: Value -= Step(); break;
        case direction::Right: Value += Step(); break;
        default:
            break;
        }
        break;
    case orientation::Vertical:
        switch (dir) {
        case direction::Down: Value -= Step(); break;
        case direction::Up: Value += Step(); break;
        default:
            break;
        }
        break;
    }
}

} // namespace ui
