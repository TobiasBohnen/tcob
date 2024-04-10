// Copyright (c) 2023 Tobias Bohnen
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
    , Value {{[&](i32 val) -> i32 {
        return std::clamp(val, Min(), Max());
    }}}
{
    Min.Changed.connect([&](auto const& val) {
        Value = std::min(val, Value());
        force_redraw(get_name() + ": Min changed");
    });
    Min(0);
    Max.Changed.connect([&](auto const& val) {
        Value = std::max(val, Value());
        force_redraw(get_name() + ": Max changed");
    });
    Max(100);
    Step.Changed.connect([&](auto const&) { force_redraw(get_name() + ": Step changed"); });
    Step(5);
    Value.Changed.connect([&](auto const&) { force_redraw(get_name() + ": Value changed"); });
    Value(0);

    Class("spinner");
}

void spinner::on_paint(widget_painter& painter)
{
    if (auto const* style {get_style<spinner::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        // text
        if (style->Text.Font) {
            painter.draw_text(style->Text, rect, std::to_string(Value()));
        }

        // arrows
        auto const& flags {get_flags()};
        auto*       normalArrow {get_sub_style<nav_arrows_style>(style->NavArrowClass, {})};
        auto*       hoverArrow {get_sub_style<nav_arrows_style>(style->NavArrowClass, {.Hover = true})};
        auto*       activeArrow {get_sub_style<nav_arrows_style>(style->NavArrowClass, {.Active = true})};
        if (_hoverArrow == arrow::None) {
            painter.draw_nav_arrows(normalArrow->NavArrow, normalArrow->NavArrow, rect);
        } else if (_hoverArrow == arrow::Increase) {
            painter.draw_nav_arrows(flags.Active ? activeArrow->NavArrow : hoverArrow->NavArrow, normalArrow->NavArrow, rect);
        } else if (_hoverArrow == arrow::Decrease) {
            painter.draw_nav_arrows(normalArrow->NavArrow, flags.Active ? activeArrow->NavArrow : hoverArrow->NavArrow, rect);
        }
    }
}

void spinner::on_mouse_hover(input::mouse::motion_event& ev)
{
    ev.Handled = true;

    if (auto const* style {get_style<spinner::style>()}) {
        rect_f const rect {get_global_content_bounds()};
        auto*        normalArrow {get_sub_style<nav_arrows_style>(style->NavArrowClass, {})};
        rect_f const navRect {normalArrow->NavArrow.calc(rect)};
        if (navRect.contains(ev.Position)) {
            if (ev.Position.Y <= navRect.get_center().Y) {
                if (_hoverArrow != arrow::Increase) {
                    _hoverArrow = arrow::Increase;
                    force_redraw(get_name() + ": arrow hover changed");
                }
            } else {
                if (_hoverArrow != arrow::Decrease) {
                    _hoverArrow = arrow::Decrease;
                    force_redraw(get_name() + ": arrow hover changed");
                }
            }

            return;
        }
    }

    if (_hoverArrow != arrow::None) {
        _hoverArrow = arrow::None;
        force_redraw(get_name() + ": arrow hover changed");
    }
}

void spinner::on_mouse_down(input::mouse::button_event& /* ev */)
{
    if (_hoverArrow == arrow::Increase) {
        Value += Step();
    } else if (_hoverArrow == arrow::Decrease) {
        Value -= Step();
    }
}

void spinner::on_mouse_wheel(input::mouse::wheel_event& ev)
{
    if (ev.Scroll.Y > 0) {
        Value += Step();
    } else if (ev.Scroll.Y < 0) {
        Value -= Step();
    }
}

void spinner::on_update(milliseconds /*deltaTime*/)
{
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
