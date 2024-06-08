// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Checkbox.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

checkbox::checkbox(init const& wi)
    : widget {wi}
{
    Checked.Changed.connect([&](auto const&) { on_checked_changed(); });

    Class("checkbox");
}

void checkbox::on_paint(widget_painter& painter)
{
    if (auto const* style {get_style<checkbox::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        if (Checked) {
            // tick
            painter.draw_tick(style->Tick, rect);
        }
    }
}

void checkbox::on_update(milliseconds /*deltaTime*/)
{
}

void checkbox::on_checked_changed()
{
    force_redraw(get_name() + ": Checked changed");
}

void checkbox::on_click()
{
    Checked = !Checked;
}

auto checkbox::get_attributes() const -> widget_attributes
{
    widget_attributes retValue {{"checked", Checked()}};
    auto const        base {widget::get_attributes()};
    retValue.insert(base.begin(), base.end());
    return retValue;
}

auto checkbox::get_flags() -> flags
{
    auto retValue {widget::get_flags()};
    retValue.Checked = Checked;
    return retValue;
}

} // namespace ui
