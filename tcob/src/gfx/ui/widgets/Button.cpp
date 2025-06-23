// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Button.hpp"

#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void button::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    text_element::Transition(target.Text, left.Text, right.Text, step);
}

button::button(init const& wi)
    : widget {wi}
{
    Label.Changed.connect([this](auto const&) { request_redraw(this->name() + ": Label changed"); });
    Icon.Changed.connect([this](auto const&) { request_redraw(this->name() + ": Icon changed"); });

    Class("button");
}

void button::on_draw(widget_painter& painter)
{
    rect_f rect {draw_background(_style, painter)};

    scissor_guard const guard {painter, this};

    painter.draw_text_and_icon(_style.Text, rect, Label, Icon);
}

void button::on_update(milliseconds /* deltaTime */)
{
}

void button::on_animation_step(string const& val)
{
    Icon.mut_ref().Region = val;
    request_redraw(this->name() + ": Animation Frame changed ");
}

auto button::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["label"] = *Label;

    return retValue;
}

} // namespace ui
