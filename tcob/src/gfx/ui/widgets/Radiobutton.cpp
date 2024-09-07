// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Radiobutton.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::gfx::ui {

radio_button::radio_button(init const& wi)
    : widget {wi}
{
    Checked.Changed.connect([&](auto const&) { on_checked_changed(); });

    Class("radio_button");
}

void radio_button::on_paint(widget_painter& painter)
{
    if (auto const* style {get_style<radio_button::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, true);

        scissor_guard const guard {painter, this};

        if (Checked) {
            // tick
            painter.draw_tick(style->Tick, rect);
        }
    }
}

void radio_button::on_update(milliseconds /*deltaTime*/)
{
}

void radio_button::on_checked_changed()
{
    if (Checked) {
        for (auto const& w : get_parent()->get_widgets()) {
            if (w.get() != this) {
                if (auto rb {std::dynamic_pointer_cast<radio_button>(w)}) {
                    rb->Checked = false;
                }
            }
        }
    }

    force_redraw(get_name() + ": Checked changed");
}

void radio_button::on_click()
{
    if (!Checked) {
        Checked = !Checked;
    }
}

auto radio_button::get_attributes() const -> widget_attributes
{
    widget_attributes retValue {{"checked", Checked()}};
    auto const        base {widget::get_attributes()};
    retValue.insert(base.begin(), base.end());
    return retValue;
}

auto radio_button::get_flags() -> widget_flags
{
    auto retValue {widget::get_flags()};
    retValue.Checked = Checked;
    return retValue;
}

} // namespace ui
