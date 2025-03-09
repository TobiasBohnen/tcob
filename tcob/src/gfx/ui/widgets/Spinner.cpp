// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Spinner.hpp"

#include <algorithm>
#include <string>

#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void spinner::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    text_element::Transition(target.Text, left.Text, right.Text, step);
}

spinner::spinner(init const& wi)
    : widget {wi}
    , Min {{[this](i32 val) -> i32 { return std::min(val, Max()); }}}
    , Max {{[this](i32 val) -> i32 { return std::max(val, Min()); }}}
    , Value {{[this](i32 val) -> i32 { return std::clamp(val, Min(), Max()); }}}
{
    Min.Changed.connect([this](auto const& val) {
        Value = std::min(val, Value());
        force_redraw(this->name() + ": Min changed");
    });
    Min(0);
    Max.Changed.connect([this](auto const& val) {
        Value = std::max(val, Value());
        force_redraw(this->name() + ": Max changed");
    });
    Max(100);
    Step.Changed.connect([this](auto const&) { force_redraw(this->name() + ": Step changed"); });
    Step(5);
    Value.Changed.connect([this](auto const&) { force_redraw(this->name() + ": Value changed"); });
    Value(0);

    Class("spinner");
}

void spinner::on_paint(widget_painter& painter)
{
    update_style(_style);

    rect_f rect {Bounds()};

    // background
    painter.draw_background_and_border(_style, rect, false);

    scissor_guard const guard {painter, this};

    // arrows
    auto const&      fls {flags()};
    nav_arrows_style incArrowStyle {};
    update_sub_style(incArrowStyle, 0, _style.NavArrowClass, {.Active = fls.Active && _hoverArrow == arrow::Increase, .Hover = !fls.Active && _hoverArrow == arrow::Increase});
    nav_arrows_style decArrowStyle {};
    update_sub_style(decArrowStyle, 1, _style.NavArrowClass, {.Active = fls.Active && _hoverArrow == arrow::Decrease, .Hover = !fls.Active && _hoverArrow == arrow::Decrease});
    _rectCache = painter.draw_nav_arrows(incArrowStyle.NavArrow, decArrowStyle.NavArrow, rect);

    // text
    if (_style.Text.Font) {
        painter.draw_text(_style.Text, {rect.left(), rect.top(), rect.width() - _rectCache.first.width(), rect.height()}, std::to_string(Value()));
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
    auto const mp {global_to_local(ev.Position)};
    if (_rectCache.first.contains(mp)) {
        if (_hoverArrow != arrow::Increase) {
            _hoverArrow = arrow::Increase;
            force_redraw(this->name() + ": arrow hover changed");
        }
    } else if (_rectCache.second.contains(mp)) {
        if (_hoverArrow != arrow::Decrease) {
            _hoverArrow = arrow::Decrease;
            force_redraw(this->name() + ": arrow hover changed");
        }
    } else if (_hoverArrow != arrow::None) {
        _hoverArrow = arrow::None;
        force_redraw(this->name() + ": arrow hover changed");
    }

    ev.Handled = true;
}

void spinner::on_mouse_down(input::mouse::button_event const& ev)
{
    if (_hoverArrow == arrow::None) { return; }

    if (_hoverArrow == arrow::Increase) {
        Value += Step();
    } else if (_hoverArrow == arrow::Decrease) {
        Value -= Step();
    }

    ev.Handled = true;
    _mouseDown = true;
    _holdTime.restart();
}

void spinner::on_mouse_up(input::mouse::button_event const& /* ev */)
{
    _mouseDown = false;
    _holdCount = 1;
}

void spinner::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    if (!is_focused()) { return; }

    if (ev.Scroll.Y > 0) {
        Value += Step();
    } else if (ev.Scroll.Y < 0) {
        Value -= Step();
    }
}

void spinner::on_update(milliseconds /*deltaTime*/)
{
    if (_mouseDown && _holdTime.elapsed_milliseconds() > (250.f / _holdCount)) {
        if (_hoverArrow == arrow::Increase) {
            Value += Step();
        } else if (_hoverArrow == arrow::Decrease) {
            Value -= Step();
        }
        _holdTime.restart();
        _holdCount += 0.1f;
    }
}

auto spinner::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["min"]   = Min();
    retValue["max"]   = Max();
    retValue["value"] = Value();
    retValue["step"]  = Step();

    return retValue;
}

} // namespace ui
