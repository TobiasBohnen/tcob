// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/charting/Legend.hpp"

#include <algorithm>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/charting/Charting.hpp"
#include "tcob/gfx/ui/component/StyleElements.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui::charts {

void legend::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.Text.lerp(from.Text, to.Text, step);
}

legend::legend(init const& wi)
    : widget {wi}
{
    Class("legend");
    For.Changed.connect([&](auto const& /* val */) -> void {
        queue_redraw();
    });
}

void legend::on_draw(widget_painter& painter)
{
    rect_f const         rect {draw_base(_style, painter)};
    scoped_scissor const guard {painter, this};
    if (!For) { return; }
    if (!_style.Text.Font) { return; }

    auto const legendDefs {For->legend()};
    if (legendDefs.empty()) { return; }

    auto& canvas {painter.canvas()};

    bool const isVertical {bounds_orientation() == orientation::Vertical};
    f32 const  itemSize {(isVertical ? rect.height() : rect.width()) / static_cast<f32>(legendDefs.size())};
    point_f    pos {rect.top_left()};

    f32 const squareSize {std::min(isVertical ? rect.width() / 4.0f : itemSize / 4.0f,
                                   (isVertical ? itemSize : rect.height()) / 2.0f)};
    f32 const outlineWidth {std::max(1.0f, squareSize / 10.0f)};
    f32 const markerSize {std::max(1.0f, squareSize / 5.0f)};

    for (auto const& [name, color, marker] : legendDefs) {
        canvas.set_fill_style(color);
        canvas.begin_path();
        canvas.rect({{pos.X, pos.Y + (((isVertical ? itemSize : rect.height()) - squareSize) / 2.0f)},
                     {squareSize, squareSize}});
        canvas.fill();

        point_f const markerPos {
            pos.X + (squareSize / 2.0f),
            pos.Y + (((isVertical ? itemSize : rect.height()) - squareSize) / 2.0f) + (squareSize / 2.0f)};
        draw_chart_marker(canvas, markerPos, marker, marker.Color.value_or(color), markerSize, marker.OutlineColor.value_or(colors::Black), outlineWidth);

        rect_f const textBounds {
            pos.X + squareSize, pos.Y,
            (isVertical ? rect.width() : itemSize) - squareSize,
            isVertical ? itemSize : rect.height()};
        painter.draw_text(_style.Text, textBounds, name);

        if (isVertical) {
            pos.Y += itemSize;
        } else {
            pos.X += itemSize;
        }
    }
}

void legend::on_update(milliseconds /*deltaTime*/)
{
}

}
