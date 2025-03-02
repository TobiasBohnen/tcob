// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Radiobutton.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::gfx::ui {

void radio_button::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    element::tick::Transition(target.Tick, left.Tick, right.Tick, step);
}

radio_button::radio_button(init const& wi)
    : widget {wi}
{
    Checked.Changed.connect([this](auto const&) { on_checked_changed(); });

    Class("radio_button");
}

void radio_button::on_paint(widget_painter& painter)
{
    update_style(_style);

    rect_f rect {Bounds()};

    // background
    painter.draw_background_and_border(_style, rect, true);

    scissor_guard const guard {painter, this};

    if (Checked()) {
        // tick
        painter.draw_tick(_style.Tick, rect);
    }
}

void radio_button::on_update(milliseconds /*deltaTime*/)
{
}

void radio_button::on_checked_changed()
{
    if (Checked()) {
        for (auto const& w : parent()->widgets()) {
            if (w.get() != this) {
                if (auto rb {std::dynamic_pointer_cast<radio_button>(w)}) {
                    rb->Checked = false;
                }
            }
        }
    }

    force_redraw(this->name() + ": Checked changed");
}

void radio_button::on_click()
{
    if (!Checked()) {
        Checked = !Checked();
    }
}

auto radio_button::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["checked"] = Checked();

    return retValue;
}

auto radio_button::flags() -> widget_flags
{
    auto retValue {widget::flags()};
    retValue.Checked = Checked();
    return retValue;
}

} // namespace ui
