// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/charting/Legend.hpp"

#include <algorithm>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
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
    rect_f const         rect {draw_background(_style, painter)};
    scoped_scissor const guard {painter, this};
    if (!For) { return; }
    if (!_style.Text.Font) { return; }
    auto&      canvas {painter.canvas()};
    auto const legendDefs {(*For)->legend()};
    if (legendDefs.empty()) { return; }

    bool const isVertical {get_orientation() == orientation::Vertical};
    f32 const  itemSize {(isVertical ? rect.height() : rect.width()) / static_cast<f32>(legendDefs.size())};
    f32 const  markerSize {std::min(isVertical ? rect.width() / 2 : itemSize / 2,
                                   (isVertical ? itemSize : rect.height()) * 0.6f)};
    point_f    pos {rect.top_left()};

    for (auto const& [name, color] : legendDefs) {
        canvas.set_fill_style(color);
        canvas.begin_path();
        canvas.rect({{pos.X, pos.Y + (((isVertical ? itemSize : rect.height()) - markerSize) * 0.5f)},
                     {markerSize, markerSize}});
        canvas.fill();

        rect_f const textBounds {
            pos.X + markerSize,
            pos.Y,
            (isVertical ? rect.width() : itemSize) - markerSize,
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
