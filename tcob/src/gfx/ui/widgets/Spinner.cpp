// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Spinner.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

spinner::spinner(init const& wi)
    : widget {wi}
    , Min {{[&](i32 val) -> i32 { return std::min(val, Max()); }}}
    , Max {{[&](i32 val) -> i32 { return std::max(val, Min()); }}}
    , Value {{[&](i32 val) -> i32 { return std::clamp(val, Min(), Max()); }}}
{
    Min.Changed.connect([&](auto const& val) {
        Value = std::min(val, Value());
        force_redraw(this->name() + ": Min changed");
    });
    Min(0);
    Max.Changed.connect([&](auto const& val) {
        Value = std::max(val, Value());
        force_redraw(this->name() + ": Max changed");
    });
    Max(100);
    Step.Changed.connect([&](auto const&) { force_redraw(this->name() + ": Step changed"); });
    Step(5);
    Value.Changed.connect([&](auto const&) { force_redraw(this->name() + ": Value changed"); });
    Value(0);

    Class("spinner");
}

void spinner::on_paint(widget_painter& painter)
{
    if (auto const* style {current_style<spinner::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        // arrows
        auto const& flags {get_flags()};
        auto const* normalArrow {get_sub_style<nav_arrows_style>(style->NavArrowClass, {})};
        auto const* hoverArrow {get_sub_style<nav_arrows_style>(style->NavArrowClass, {.Hover = true})};
        auto const* activeArrow {get_sub_style<nav_arrows_style>(style->NavArrowClass, {.Active = true})};
        f32 const   arrowWidth {normalArrow->NavArrow.Size.Width.calc(rect.width())}; // FIXME: max arrow style width
        if (_hoverArrow == arrow::None) {
            painter.draw_nav_arrows(normalArrow->NavArrow, normalArrow->NavArrow, rect);
        } else if (_hoverArrow == arrow::Increase) {
            painter.draw_nav_arrows(flags.Active ? activeArrow->NavArrow : hoverArrow->NavArrow, normalArrow->NavArrow, rect);
        } else if (_hoverArrow == arrow::Decrease) {
            painter.draw_nav_arrows(normalArrow->NavArrow, flags.Active ? activeArrow->NavArrow : hoverArrow->NavArrow, rect);
        }

        // text
        if (style->Text.Font) {
            painter.draw_text(style->Text, {rect.left(), rect.top(), rect.width() - arrowWidth, rect.height()}, std::to_string(Value()));
        }
    }
}

void spinner::on_mouse_leave()
{
    _mouseDown = false;
    _holdCount = 1;
}

void spinner::on_mouse_hover(input::mouse::motion_event const& ev)
{
    ev.Handled = true;

    if (auto const* style {current_style<spinner::style>()}) {
        rect_f const rect {global_content_bounds()};
        auto const*  normalArrow {get_sub_style<nav_arrows_style>(style->NavArrowClass, {})};
        rect_f const navRect {normalArrow->NavArrow.calc(rect)};
        if (navRect.contains(ev.Position)) {
            if (ev.Position.Y <= navRect.center().Y) {
                if (_hoverArrow != arrow::Increase) {
                    _hoverArrow = arrow::Increase;
                    force_redraw(this->name() + ": arrow hover changed");
                }
            } else {
                if (_hoverArrow != arrow::Decrease) {
                    _hoverArrow = arrow::Decrease;
                    force_redraw(this->name() + ": arrow hover changed");
                }
            }

            return;
        }
    }

    if (_hoverArrow != arrow::None) {
        _hoverArrow = arrow::None;
        force_redraw(this->name() + ": arrow hover changed");
    }
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

auto spinner::get_attributes() const -> widget_attributes
{
    widget_attributes retValue {{"min", Min()},
                                {"max", Max()},
                                {"step", Step()},
                                {"value", Value()}};
    auto const        base {widget::get_attributes()};
    retValue.insert(base.begin(), base.end());
    return retValue;
}

} // namespace ui
