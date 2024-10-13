// Copyright (c) 2024 Tobias Bohnen
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
    Checked.Changed.connect([&](auto const&) { on_checked_changed(); });

    Class("toggle");
}

void toggle::on_paint(widget_painter& painter)
{
    if (auto const* style {get_style<toggle::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        // tick
        f32 const tickWidth {rect.width() / 2};

        rect.Size.Width = tickWidth;
        rect.Position.X += tickWidth * _tween.get_current_value();
        painter.draw_tick(style->Tick, rect);
    }
}

void toggle::on_update(milliseconds deltaTime)
{
    _tween.update(deltaTime);
}

void toggle::on_checked_changed()
{
    if (auto const* style {get_style<toggle::style>()}) {
        if (Checked) {
            _tween.start(1.0f, style->Delay * (1.0f - _tween.get_current_value()));
        } else {
            _tween.start(0.0f, style->Delay * _tween.get_current_value());
        }
    } else {
        _tween.reset(Checked ? 1.0f : 0.0f);
    }
}

void toggle::on_click()
{
    Checked = !Checked;
}

auto toggle::get_attributes() const -> widget_attributes
{
    widget_attributes retValue {{"checked", Checked()}};
    auto const        base {widget::get_attributes()};
    retValue.insert(base.begin(), base.end());
    return retValue;
}

auto toggle::get_flags() -> widget_flags
{
    auto retValue {widget::get_flags()};
    retValue.Checked = _tween.get_current_value() >= 0.5f;
    return retValue;
}

} // namespace ui
