// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Toggle.hpp"

#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {

void toggle::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    element::tick::Transition(target.Tick, left.Tick, right.Tick, step);
}

toggle::toggle(init const& wi)
    : widget {wi}
    , _tween {*this}
{
    Checked.Changed.connect([this](auto const&) { on_checked_changed(); });

    Class("toggle");
}

void toggle::on_paint(widget_painter& painter)
{
    update_style(_style);

    rect_f rect {Bounds()};

    // background
    painter.draw_background_and_border(_style, rect, false);

    scissor_guard const guard {painter, this};

    // tick
    f32 const tickWidth {rect.width() / 2};
    rect.Size.Width = tickWidth;
    rect.Position.X += tickWidth * _tween.current_value();
    painter.draw_tick(_style.Tick, rect);
}

void toggle::on_update(milliseconds deltaTime)
{
    _tween.update(deltaTime);
}

void toggle::on_checked_changed()
{
    if (Checked()) {
        _tween.start(1.0f, _style.Delay * (1.0f - _tween.current_value()));
    } else {
        _tween.start(0.0f, _style.Delay * _tween.current_value());
    }
}

void toggle::on_click()
{
    Checked = !Checked();
}

auto toggle::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["checked"] = Checked();

    return retValue;
}

auto toggle::flags() -> widget_flags
{
    auto retValue {widget::flags()};
    retValue.Checked = _tween.current_value() >= 0.5f;
    return retValue;
}

} // namespace ui
