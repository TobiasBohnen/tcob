// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Checkbox.hpp"

#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void checkbox::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.Tick.lerp(from.Tick, to.Tick, step);
}

checkbox::checkbox(init const& wi)
    : widget {wi}
{
    Checked.Changed.connect([this](auto const&) { on_checked_changed(); });

    Class("checkbox");
}

void checkbox::on_draw(widget_painter& painter)
{
    rect_f const rect {draw_background(_style, painter)};

    scissor_guard const guard {painter, this};

    if (Checked) {
        // tick
        painter.draw_tick(_style.Tick, rect);
    }
}

void checkbox::on_update(milliseconds /*deltaTime*/)
{
}

void checkbox::on_checked_changed()
{
    queue_redraw();
}

void checkbox::on_click()
{
    Checked = !Checked;
}

auto checkbox::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["checked"] = *Checked;

    return retValue;
}

auto checkbox::flags() -> widget_flags
{
    auto retValue {widget::flags()};
    retValue.Checked = Checked;
    return retValue;
}

}
