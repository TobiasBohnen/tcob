// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Spinner.hpp"

#include <algorithm>
#include <string>

#include "tcob/core/Common.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void spinner::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.Text.lerp(from.Text, to.Text, step);
}

spinner::spinner(init const& wi)
    : widget {wi}
    , Min {{[this](i32 val) -> i32 { return std::min(val, *Max); }}}
    , Max {{[this](i32 val) -> i32 { return std::max(val, *Min); }}}
    , Value {{[this](i32 val) -> i32 { return std::clamp(val, *Min, *Max); }}}
{
    Min.Changed.connect([this](auto const& val) {
        Value = std::min(val, *Value);
        queue_redraw();
    });
    Min(0);
    Max.Changed.connect([this](auto const& val) {
        Value = std::max(val, *Value);
        queue_redraw();
    });
    Max(100);
    Step.Changed.connect([this](auto const&) { queue_redraw(); });
    Step(5);
    Value.Changed.connect([this](auto const&) { queue_redraw(); });
    Value(0);

    Class("spinner");
}

void spinner::on_draw(widget_painter& painter)
{
    rect_f const rect {draw_background(_style, painter)};

    scissor_guard const guard {painter, this};

    // arrows
    auto const&      fls {flags()};
    nav_arrows_style incArrowStyle {};
    prepare_sub_style(incArrowStyle, 0, _style.NavArrowClass, {.Active = fls.Active && _hoverArrow == arrow::Increase, .Hover = !fls.Active && _hoverArrow == arrow::Increase});
    nav_arrows_style decArrowStyle {};
    prepare_sub_style(decArrowStyle, 1, _style.NavArrowClass, {.Active = fls.Active && _hoverArrow == arrow::Decrease, .Hover = !fls.Active && _hoverArrow == arrow::Decrease});

    rect_f incRect {rect};
    incRect.Size.Height /= 2;
    _rectCache.first = painter.draw_nav_arrow(incArrowStyle.NavArrow, incRect, direction::Up);
    rect_f decRect {incRect};
    decRect.Position.Y += decRect.height();
    _rectCache.second = painter.draw_nav_arrow(decArrowStyle.NavArrow, decRect, direction::Down);

    // text
    if (_style.Text.Font) {
        painter.draw_text(_style.Text, {rect.left(), rect.top(), rect.width() - _rectCache.first.width(), rect.height()}, std::to_string(*Value));
    }
}

void spinner::on_mouse_leave()
{
    _mouseDown  = false;
    _holdCount  = 1;
    _hoverArrow = arrow::None;
}

void spinner::on_mouse_hover(input::mouse::motion_event const& ev)
{
    auto const mp {screen_to_local(*this, ev.Position)};
    if (_rectCache.first.contains(mp)) {
        if (_hoverArrow != arrow::Increase) {
            _hoverArrow = arrow::Increase;
            ev.Handled  = true;
        }
    } else if (_rectCache.second.contains(mp)) {
        if (_hoverArrow != arrow::Decrease) {
            _hoverArrow = arrow::Decrease;
            ev.Handled  = true;
        }
    } else if (_hoverArrow != arrow::None) {
        _hoverArrow = arrow::None;
        ev.Handled  = true;
    }
}

void spinner::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (_hoverArrow == arrow::None) { return; }

    if (_hoverArrow == arrow::Increase) {
        Value += *Step;
    } else if (_hoverArrow == arrow::Decrease) {
        Value -= *Step;
    }

    ev.Handled = true;
    _mouseDown = true;
    _holdTime.restart();
}

void spinner::on_mouse_button_up(input::mouse::button_event const& /* ev */)
{
    _mouseDown = false;
    _holdCount = 1;
}

void spinner::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    if (!is_focused()) { return; }

    if (ev.Scroll.Y > 0) {
        Value += *Step;
    } else if (ev.Scroll.Y < 0) {
        Value -= *Step;
    }

    ev.Handled = true;
}

void spinner::on_key_down(input::keyboard::event const& ev)
{
    auto const& controls {form().Controls};
    if (ev.Keyboard->is_key_down(controls->ActivateKey)) {
        if (ev.KeyCode == controls->NavDownKey) {
            Value -= *Step;
            ev.Handled = true;
        } else if (ev.KeyCode == controls->NavUpKey) {
            Value += *Step;
            ev.Handled = true;
        }
    }
}

void spinner::on_controller_button_down(input::controller::button_event const& ev)
{
    auto const& controls {form().Controls};
    if (ev.Controller->is_button_pressed(controls->ActivateButton)) {
        if (ev.Button == controls->NavDownButton) {
            Value -= *Step;
            ev.Handled = true;
        } else if (ev.Button == controls->NavUpButton) {
            Value += *Step;
            ev.Handled = true;
        }
    }
}

void spinner::on_update(milliseconds /*deltaTime*/)
{
    if (_mouseDown && _holdTime.elapsed_milliseconds() > (250.0f / _holdCount)) {
        if (_hoverArrow == arrow::Increase) {
            Value += *Step;
        } else if (_hoverArrow == arrow::Decrease) {
            Value -= *Step;
        }
        _holdTime.restart();
        _holdCount += 0.1f;
    }
}

auto spinner::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["min"]   = *Min;
    retValue["max"]   = *Max;
    retValue["value"] = *Value;
    retValue["step"]  = *Step;

    return retValue;
}

} // namespace ui
