// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Progressbar.hpp"

#include <algorithm>

#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void progress_bar::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    target.Bar = bar_element::Lerp(left.Bar, right.Bar, step);
}

progress_bar::progress_bar(init const& wi)
    : widget {wi}
    , Min {{[this](i32 val) -> i32 { return std::min(val, Max()); }}}
    , Max {{[this](i32 val) -> i32 { return std::max(val, Min()); }}}
    , Value {{[this](i32 val) -> i32 { return std::clamp(val, Min(), Max()); }}}
    , _tween {*this}
{
    Min.Changed.connect([this](auto val) {
        Value = std::max(val, Value());
        on_value_changed(Value());
        force_redraw(this->name() + ": Min changed");
    });
    Min(0);
    Max.Changed.connect([this](auto val) {
        Value = std::min(val, Value());
        on_value_changed(Value());
        force_redraw(this->name() + ": Max changed");
    });
    Max(100);
    Value.Changed.connect([this](auto val) { on_value_changed(val); });
    Value(Min());

    Class("progress_bar");
}

void progress_bar::on_paint(widget_painter& painter)
{
    update_style(_style);

    rect_f rect {Bounds()};

    // background
    painter.draw_background_and_border(_style, rect, false);

    scissor_guard const guard {painter, this};

    // bar
    i32 const numBlocks {10};

    (void)painter.draw_bar(
        _style.Bar,
        rect,
        {.Orientation = current_orientation(),
         .Inverted    = false,
         .Position    = bar_element::position::CenterOrMiddle,
         .BlockCount  = numBlocks,
         .Fraction    = _tween.current_value()});
}

void progress_bar::on_update(milliseconds deltaTime)
{
    _tween.update(deltaTime);
}

void progress_bar::on_value_changed(i32 newVal)
{
    f32 const newFrac {static_cast<f32>(newVal - Min()) / (Max() - Min())};
    _tween.start(newFrac, _style.Bar.Delay);
}

auto progress_bar::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["min"]   = Min();
    retValue["max"]   = Max();
    retValue["value"] = Value();

    return retValue;
}

} // namespace ui
