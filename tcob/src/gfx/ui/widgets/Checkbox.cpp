// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Checkbox.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

void checkbox::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    element::tick::Transition(target.Tick, left.Tick, right.Tick, step);
}

checkbox::checkbox(init const& wi)
    : widget {wi}
{
    Checked.Changed.connect([this](auto const&) { on_checked_changed(); });

    Class("checkbox");
}

void checkbox::on_paint(widget_painter& painter)
{
    get_style(_style);

    rect_f rect {Bounds()};

    // background
    painter.draw_background_and_border(_style, rect, false);

    scissor_guard const guard {painter, this};

    if (Checked()) {
        // tick
        painter.draw_tick(_style.Tick, rect);
    }
}

void checkbox::on_update(milliseconds /*deltaTime*/)
{
}

void checkbox::on_checked_changed()
{
    force_redraw(this->name() + ": Checked changed");
}

void checkbox::on_click()
{
    Checked = !Checked();
}

auto checkbox::attributes() const -> widget_attributes
{
    widget_attributes retValue {{"checked", Checked()}};
    auto const        base {widget::attributes()};
    retValue.insert(base.begin(), base.end());
    return retValue;
}

auto checkbox::flags() -> widget_flags
{
    auto retValue {widget::flags()};
    retValue.Checked = Checked();
    return retValue;
}

} // namespace ui
