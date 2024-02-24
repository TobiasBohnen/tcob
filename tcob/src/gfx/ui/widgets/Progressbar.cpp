// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Progressbar.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

progress_bar::progress_bar(init const& wi)
    : widget {wi}
    , Min {{[&](i32 val) -> i32 { return std::min(val, Max()); }}}
    , Max {{[&](i32 val) -> i32 { return std::max(val, Min()); }}}
    , Value {{[&](i32 val) -> i32 { return std::clamp(val, Min(), Max()); }}}
    , _tween {*this}
{
    Min.Changed.connect([&](auto val) { Value = std::min(val, Value()); });
    Min(0);
    Max.Changed.connect([&](auto val) { Value = std::max(val, Value()); });
    Max(100);
    Value.Changed.connect([&](auto val) { on_value_changed(val); });
    Value(Min());

    Class("progress_bar");
}

void progress_bar::on_paint(widget_painter& painter)
{
    if (auto const style {get_style<progress_bar::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        // bar
        i32 const numBlocks {10};

        (void)painter.draw_bar(
            style->Bar,
            rect,
            {.Orientation = get_orientation(),
             .Inverted    = false,
             .Alignment   = element::bar::alignment::CenterOrMiddle,
             .BlockCount  = numBlocks,
             .Fraction    = _tween.get_value()});
    }
}

void progress_bar::on_update(milliseconds deltaTime)
{
    _tween.update(deltaTime);
}

void progress_bar::on_value_changed(i32 newVal)
{
    f32 const newFrac {static_cast<f32>(newVal - Min) / (Max - Min)};
    if (auto const style {get_style<progress_bar::style>()}) {
        _tween.start(newFrac, style->Bar.Delay);
    } else {
        _tween.reset(newFrac);
    }
}

auto progress_bar::get_attributes() const -> widget_attributes
{
    widget_attributes retValue {{"min", Min()},
                                {"max", Max()},
                                {"value", Value()}};
    auto const        base {widget::get_attributes()};
    retValue.insert(base.begin(), base.end());
    return retValue;
}

auto progress_bar::get_properties() const -> widget_attributes
{
    auto retValue {widget::get_properties()};
    retValue["value"] = Value();
    return retValue;
}

} // namespace ui
