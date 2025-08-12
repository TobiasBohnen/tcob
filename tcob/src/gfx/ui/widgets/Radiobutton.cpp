// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Radiobutton.hpp"

#include <memory>

#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::ui {

void radio_button::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.Tick.lerp(from.Tick, to.Tick, step);
}

radio_button::radio_button(init const& wi)
    : widget {wi}
{
    Checked.Changed.connect([this](auto const&) { on_checked_changed(); });

    Class("radio_button");
}

void radio_button::on_draw(widget_painter& painter)
{
    rect_f rect {draw_background(_style, painter, true)};

    scissor_guard const guard {painter, this};

    if (Checked) {
        // tick
        painter.draw_tick(_style.Tick, rect);
    }
}

void radio_button::on_update(milliseconds /*deltaTime*/)
{
}

void radio_button::on_checked_changed()
{
    if (Checked) {
        for (auto const& w : parent()->widgets()) {
            if (w.get() != this) {
                if (auto rb {std::dynamic_pointer_cast<radio_button>(w)}) {
                    rb->Checked = false;
                }
            }
        }
    }

    queue_redraw();
}

void radio_button::on_click()
{
    if (!Checked) {
        Checked = !Checked;
    }
}

auto radio_button::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["checked"] = *Checked;

    return retValue;
}

auto radio_button::flags() -> widget_flags
{
    auto retValue {widget::flags()};
    retValue.Checked = *Checked;
    return retValue;
}

} // namespace ui
