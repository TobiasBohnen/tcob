// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Toggle.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

toggle::toggle(init const& wi)
    : widget {wi}
    , _tween {*this}
{
    Enabled.Changed.connect([&](auto const&) { on_enabled_changed(); });

    Class("toggle");
}

void toggle::on_paint(widget_painter& painter)
{
    if (auto const style {get_style<toggle::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        // tick
        f32 const tickWidth {rect.Width / 2};

        rect.Width = tickWidth;
        rect.X += tickWidth * _tween.get_current_value();
        painter.draw_tick(style->Tick, rect);
    }
}

void toggle::on_update(milliseconds deltaTime)
{
    _tween.update(deltaTime);
}

void toggle::on_enabled_changed()
{
    if (auto const style {get_style<toggle::style>()}) {
        if (Enabled) {
            _tween.start(1.0f, style->Delay * (1.0f - _tween.get_current_value()));
        } else {
            _tween.start(0.0f, style->Delay * _tween.get_current_value());
        }
    } else {
        _tween.reset(Enabled ? 1.0f : 0.0f);
    }
}

void toggle::on_click()
{
    Enabled = !Enabled;
}

auto toggle::get_properties() const -> widget_attributes
{
    auto retValue {widget::get_properties()};
    retValue["enabled"] = Enabled();
    return retValue;
}

auto toggle::get_flags() -> flags
{
    auto retValue {widget::get_flags()};
    retValue.Checked = _tween.get_current_value() >= 0.5f;
    return retValue;
}

} // namespace ui
