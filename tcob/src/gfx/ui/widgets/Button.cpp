// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Button.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

button::button(init const& wi)
    : widget {wi}
{
    Label.Changed.connect([&](auto const&) { force_redraw(get_name() + ": Label changed"); });

    Class("button");
}

void button::on_paint(widget_painter& painter)
{
    if (auto const* style {get_style<button::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        // text
        if (style->Text.Font) {
            painter.draw_text(style->Text, rect, Label());
        }
    }
}

void button::on_update(milliseconds /*deltaTime*/)
{
}

auto button::get_attributes() const -> widget_attributes
{
    widget_attributes retValue {{"label", Label()}};
    auto const        base {widget::get_attributes()};
    retValue.insert(base.begin(), base.end());
    return retValue;
}

} // namespace ui
