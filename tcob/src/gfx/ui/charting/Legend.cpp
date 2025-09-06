// Copyright (c) 2025 Tobias Bohnen
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
    For.Changed.connect([&](auto const& val) -> void {
        _con = val->Drawn.connect([&] { queue_redraw(); });
        queue_redraw();
    });
}

void legend::on_draw(widget_painter& painter)
{
    rect_f const rect {draw_background(_style, painter)};

    scissor_guard const guard {painter, this};

    if (!For) { return; }
    if (!_style.Text.Font) { return; }

    auto&      canvas {painter.canvas()};
    auto const legendDefs {For->legend()};
    if (legendDefs.empty()) { return; }

    f32 const lineHeight {rect.height() / static_cast<f32>(legendDefs.size())};
    f32 const markerSize {std::min(rect.width() / 2, lineHeight * 0.6f)};

    point_f pos {rect.top_left()};

    for (auto const& [name, color] : legendDefs) {
        canvas.set_fill_style(color);
        canvas.begin_path();
        canvas.rect({{pos.X, pos.Y + ((lineHeight - markerSize) * 0.5f)},
                     {markerSize, markerSize}});
        canvas.fill();

        rect_f const textBounds {
            pos.X + markerSize,
            pos.Y,
            rect.width() - markerSize,
            lineHeight};

        painter.draw_text(_style.Text, textBounds, name);

        pos.Y += lineHeight;
    }
}

void legend::on_update(milliseconds /*deltaTime*/)
{
}

} // namespace ui
