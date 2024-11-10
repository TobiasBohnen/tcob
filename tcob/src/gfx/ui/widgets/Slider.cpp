// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Slider.hpp"

#include "tcob/core/Common.hpp"
#include "tcob/gfx/ui/Form.hpp"

namespace tcob::gfx::ui {

slider::slider(init const& wi)
    : widget {wi}
    , Min {{[&](i32 val) -> i32 { return std::min(val, Max()); }}}
    , Max {{[&](i32 val) -> i32 { return std::max(val, Min()); }}}
    , Value {{[&](i32 val) -> i32 {
        if (IncrementalChange && std::abs(Value - val) > Step) {
            return Value();
        }

        return std::clamp(val, Min(), Max());
    }}}
    , _tween {*this}
{
    Min.Changed.connect([&](auto val) {
        Value = std::max(val, Value());
        on_value_changed(Value());
        force_redraw(this->name() + ": Min changed");
    });
    Min(0);
    Max.Changed.connect([&](auto val) {
        Value = std::min(val, Value());
        on_value_changed(Value());
        force_redraw(this->name() + ": Max changed");
    });
    Max(100);
    Step.Changed.connect([&](auto) { force_redraw(this->name() + ": Step changed"); });
    Step(1);
    Value.Changed.connect([&](auto val) { on_value_changed(val); });
    Value(Min());

    Class("slider");
}

void slider::on_paint(widget_painter& painter)
{
    if (auto const* style {get_style<slider::style>()}) {
        rect_f const rect {get_content_bounds()};

        scissor_guard const guard {painter, this};

        i32 const  numBlocks {10};
        auto const orien {get_orientation()};
        auto const pos {element::bar::position::CenterOrMiddle};

        // bar
        _paintResult.Bar = painter.draw_bar(
            style->Bar,
            rect,
            {.Orientation = orien,
             .Inverted    = false,
             .Position    = pos,
             .BlockCount  = numBlocks,
             .Fraction    = _tween.get_current_value()});

        // thumb
        auto const thumbFlags {!_overThumb              ? widget_flags {}
                                   : get_flags().Active ? widget_flags {.Active = true}
                                                        : widget_flags {.Hover = true}};
        _paintResult.Thumb = painter.draw_thumb(
            get_sub_style<thumb_style>(style->ThumbClass, thumbFlags)->Thumb,
            rect,
            {.Orientation = orien, .Inverted = false, .Fraction = _tween.get_current_value()});

        if (orien == orientation::Vertical) {
            _paintResult.Thumb.Size.Width -= get_sub_style<thumb_style>(style->ThumbClass, {})->Thumb.Border.Size.calc(_paintResult.Thumb.width());
        } else if (orien == orientation::Horizontal) {
            _paintResult.Thumb.Size.Height -= get_sub_style<thumb_style>(style->ThumbClass, {})->Thumb.Border.Size.calc(_paintResult.Thumb.height());
        }
    }
}

void slider::on_key_down(input::keyboard::event const& ev)
{
    auto const& controls {get_form()->Controls};
    if (input::system::IsKeyDown(controls->ActivateKey)) {
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
    bool const overThumb {_paintResult.Thumb.contains(global_to_parent_local(ev.Position))};
    if (overThumb != _overThumb) {
        _overThumb = overThumb;
        force_redraw(this->name() + ": thumb hover change");
        ev.Handled = true;
    }
}

void slider::on_mouse_drag(input::mouse::motion_event const& ev)
{
    calculate_value(global_to_local(ev.Position));
    ev.Handled = true;
}

void slider::on_mouse_up(input::mouse::button_event const& ev)
{
    _dragOffset = point_i::Zero;
    if (_overThumb && !hit_test(point_f {ev.Position})) {
        _overThumb = false;
        force_redraw(this->name() + ": thumb hit");
        ev.Handled = true;
    }
}

void slider::on_mouse_down(input::mouse::button_event const& ev)
{
    _isDragging = false;

    if (ev.Button == get_form()->Controls->PrimaryMouseButton) {
        if (!_overThumb) {
            calculate_value(global_to_local(ev.Position));
        } else {
            _dragOffset = point_i {global_to_parent_local(ev.Position) - _paintResult.Thumb.center()};
            _isDragging = true;
        }
        ev.Handled = true;
    }
}

void slider::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
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
    auto const& controls {get_form()->Controls};
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
    f32 const   newFrac {Max != Min ? static_cast<f32>(newVal - Min) / (Max - Min) : 0.f};
    auto const* style {get_style<slider::style>()};
    if (style && !_overThumb && !_isDragging) {
        _tween.start(newFrac, style->Bar.Delay);
    } else {
        _tween.reset(newFrac);
    }
}

void slider::calculate_value(point_f mp)
{
    rect_f const rect {_paintResult.Bar};
    f32          frac {0.0f};

    switch (get_orientation()) {
    case orientation::Horizontal: {
        f32 const tw {_paintResult.Thumb.width()};
        frac = (mp.X - _dragOffset.X - (tw / 2)) / (rect.width() - tw);
    } break;
    case orientation::Vertical: {
        f32 const th {_paintResult.Thumb.height()};
        frac = 1.0f - ((mp.Y - _dragOffset.Y - (th / 2)) / (rect.height() - th));
    } break;
    }

    i32 const val {static_cast<i32>(Min + ((Max - Min + 1) * frac))};
    Value = helper::round_to_multiple(val, Step());

    if (!_overThumb) {
        _overThumb = true;
        force_redraw(this->name() + ": thumb move after value change");
    }
}

auto slider::get_attributes() const -> widget_attributes
{
    widget_attributes retValue {{"min", Min()},
                                {"max", Max()},
                                {"step", Step()},
                                {"value", Value()}};
    auto const        base {widget::get_attributes()};
    retValue.insert(base.begin(), base.end());
    return retValue;
}

void slider::handle_dir_input(direction dir)
{
    switch (get_orientation()) {
    case orientation::Horizontal:
        switch (dir) {
        case direction::Left:
            Value -= Step();
            break;
        case direction::Right:
            Value += Step();
            break;
        default:
            break;
        }
        break;
    case orientation::Vertical:
        switch (dir) {
        case direction::Down:
            Value -= Step();
            break;
        case direction::Up:
            Value += Step();
            break;
        default:
            break;
        }
        break;
    }
}

} // namespace ui
