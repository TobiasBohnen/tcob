// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Progressbar.hpp"

#include <algorithm>
#include <tuple>

#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void progress_bar::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.Bar.lerp(from.Bar, to.Bar, step);
}

progress_bar::progress_bar(init const& wi)
    : widget {wi}
    , Min {{[this](i32 val) -> i32 { return std::min(val, *Max); }}}
    , Max {{[this](i32 val) -> i32 { return std::max(val, *Min); }}}
    , Value {{[this](i32 val) -> i32 { return std::clamp(val, *Min, *Max); }}}
{
    _tween.Changed.connect([this] {
        queue_redraw();
    });
    Min.Changed.connect([this](auto val) {
        Value = std::max(val, *Value);
        on_value_changed(*Value);
        queue_redraw();
    });
    Min(0);
    Max.Changed.connect([this](auto val) {
        Value = std::min(val, *Value);
        on_value_changed(*Value);
        queue_redraw();
    });
    Max(100);
    Value.Changed.connect([this](auto val) { on_value_changed(val); });
    Value(*Min);

    Class("progress_bar");
}

void progress_bar::on_draw(widget_painter& painter)
{
    rect_f const rect {draw_background(_style, painter)};

    scissor_guard const guard {painter, this};

    // bar
    std::ignore = painter.draw_bar(
        _style.Bar,
        rect,
        {.Orientation = get_orientation(),
         .Position    = bar_element::position::CenterOrMiddle,
         .Stops       = {0.0f, _tween.current_value(), 1.0f},
         .StopPattern = {bar_element::type::Low, bar_element::type::High}});
}

void progress_bar::on_update(milliseconds deltaTime)
{
    _tween.update(deltaTime);
}

void progress_bar::on_value_changed(i32 newVal)
{
    f32 const newFrac {static_cast<f32>(newVal - *Min) / (*Max - *Min)};
    _tween.start(newFrac, _style.Bar.Delay);
}

auto progress_bar::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["min"]   = *Min;
    retValue["max"]   = *Max;
    retValue["value"] = *Value;

    return retValue;
}

}
