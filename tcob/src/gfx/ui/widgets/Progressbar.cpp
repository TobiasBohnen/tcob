// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Progressbar.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

void progress_bar::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    element::bar::Transition(target.Bar, left.Bar, right.Bar, step);
}

progress_bar::progress_bar(init const& wi)
    : widget {wi}
    , Min {{[this](i32 val) -> i32 { return std::min(val, Max()); }}}
    , Max {{[this](i32 val) -> i32 { return std::max(val, Min()); }}}
    , Value {{[this](i32 val) -> i32 { return std::clamp(val, Min(), Max()); }}}
    , _tween {*this}
{
    Min.Changed.connect([this](auto val) { Value = std::min(val, Value()); });
    Min(0);
    Max.Changed.connect([this](auto val) { Value = std::max(val, Value()); });
    Max(100);
    Value.Changed.connect([this](auto val) { on_value_changed(val); });
    Value(Min());

    Class("progress_bar");
}

void progress_bar::on_paint(widget_painter& painter)
{
    get_style(_style);

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
         .Position    = element::bar::position::CenterOrMiddle,
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
    widget_attributes retValue {{"min", Min()},
                                {"max", Max()},
                                {"value", Value()}};
    auto const        base {widget::attributes()};
    retValue.insert(base.begin(), base.end());
    return retValue;
}

} // namespace ui
