// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Label.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

void label::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    element::text::Transition(target.Text, left.Text, right.Text, step);
}

label::label(init const& wi)
    : widget {wi}
{
    Label.Changed.connect([this](auto const&) {
        // TODO: translation hook
        force_redraw(this->name() + ": Label changed");
    });

    Class("label");
}

void label::on_paint(widget_painter& painter)
{
    update_style(_style);

    rect_f rect {Bounds()};

    // background
    painter.draw_background_and_border(_style, rect, false);

    scissor_guard const guard {painter, this};

    // text
    if (_style.Text.Font) {
        painter.draw_text(_style.Text, rect, Label());
    }
}

void label::on_key_down(input::keyboard::event const& ev)
{
    if (For) {
        _injector.on_key_down(For.get(), ev);
    }
}

void label::on_key_up(input::keyboard::event const& ev)
{
    if (For) {
        _injector.on_key_up(For.get(), ev);
    }
}

void label::on_mouse_enter()
{
    if (For) {
        _injector.on_mouse_enter(For.get());
    }
}

void label::on_mouse_leave()
{
    if (For) {
        _injector.on_mouse_leave(For.get());
    }
}

void label::on_mouse_hover(input::mouse::motion_event const& ev)
{
    if (For) {
        _injector.on_mouse_hover(For.get(), ev);
    }
}

void label::on_mouse_drag(input::mouse::motion_event const& ev)
{
    if (For) {
        _injector.on_mouse_drag(For.get(), ev);
    }
}

void label::on_mouse_down(input::mouse::button_event const& ev)
{
    if (For) {
        _injector.on_mouse_down(For.get(), ev);
    }
}

void label::on_mouse_up(input::mouse::button_event const& ev)
{
    if (For) {
        _injector.on_mouse_up(For.get(), ev);
    }
}

void label::on_click()
{
    if (For) {
        _injector.on_click(For.get());
    }
}

void label::on_focus_gained()
{
    if (For) {
        _injector.on_focus_gained(For.get());
    }
}

void label::on_focus_lost()
{
    if (For) {
        _injector.on_focus_lost(For.get());
    }
}

void label::on_update(milliseconds /*deltaTime*/)
{
}

auto label::attributes() const -> widget_attributes
{
    widget_attributes retValue {{"label", Label()}};
    auto const        base {widget::attributes()};
    retValue.insert(base.begin(), base.end());
    return retValue;
}

} // namespace ui
