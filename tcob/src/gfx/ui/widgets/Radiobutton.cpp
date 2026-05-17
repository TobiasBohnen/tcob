// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Radiobutton.hpp"

#include <memory>

#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/component/StyleElements.hpp"
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
    rect_f const rect {draw_base(_style, painter, true)};

    scoped_scissor const guard {painter, this};

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
                if (auto* rb {dynamic_cast<radio_button*>(w.get())}) {
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

auto radio_button::flags() -> widget_flags
{
    auto retValue {widget::flags()};
    retValue.Checked = *Checked;
    return retValue;
}

}
