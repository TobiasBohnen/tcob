// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Button.hpp"

#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void button::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.Text.lerp(from.Text, to.Text, step);
}

button::button(init const& wi)
    : widget {wi}
{
    Label.Changed.connect([this](auto const&) { queue_redraw(); });
    Icon.Changed.connect([this](auto const&) { queue_redraw(); });

    Class("button");
}

void button::on_draw(widget_painter& painter)
{
    rect_f rect {draw_background(_style, painter)};

    scissor_guard const guard {painter, this};

    painter.draw_text_and_icon(_style.Text, rect, Label, Icon, _style.IconTextOrder);
}

void button::on_update(milliseconds /* deltaTime */)
{
}

auto button::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["label"] = *Label;

    return retValue;
}

} // namespace ui
