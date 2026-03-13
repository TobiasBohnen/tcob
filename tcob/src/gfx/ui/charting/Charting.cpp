// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/charting/Charting.hpp"

#include <cmath>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui::charts {

////////////////////////////////////////////////////////////

void chart_style::Transition(chart_style& target, chart_style const& from, chart_style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.XAxisText.lerp(from.XAxisText, to.XAxisText, step);
    target.XLabelHeight = helper::lerp(from.XLabelHeight, to.XLabelHeight, step);
    target.YAxisText.lerp(from.YAxisText, to.YAxisText, step);
    target.YLabelWidth = helper::lerp(from.YLabelWidth, to.YLabelWidth, step);

    target.OutlineSize = helper::lerp(from.OutlineSize, to.OutlineSize, step);

    target.MarkerSize = helper::lerp(from.MarkerSize, to.MarkerSize, step);
}

////////////////////////////////////////////////////////////

void grid_chart_style::Transition(grid_chart_style& target, grid_chart_style const& from, grid_chart_style const& to, f64 step)
{
    chart_style::Transition(target, from, to, step);

    target.HorizontalGridLines = helper::lerp(from.HorizontalGridLines, to.HorizontalGridLines, step);
    target.VerticalGridLines   = helper::lerp(from.VerticalGridLines, to.VerticalGridLines, step);

    target.GridLineSize = helper::lerp(from.GridLineSize, to.GridLineSize, step);
    target.GridColor    = helper::lerp(from.GridColor, to.GridColor, step);

    target.TickSize = helper::lerp(from.TickSize, to.TickSize, step);
}

////////////////////////////////////////////////////////////

chart_base::chart_base(init const& wi)
    : widget {wi}
{
    auto static const calcCount {[](axis const& a) -> usize {
        if (!a.CustomLabels.empty()) { return a.CustomLabels.size(); }
        if (a.LargeStep <= 0.0f) { return 0; }
        return static_cast<usize>(std::round((a.Max - a.Min) / a.LargeStep)) + 1;
    }};

    XAxis.Changed.connect([&](axis const& axis) {
        _labelX = calcCount(axis);
        queue_redraw();
    });
    YAxis.Changed.connect([&](axis const& axis) {
        _labelY = calcCount(axis);
        queue_redraw();
    });
}

auto chart_base::x_label_count() const -> usize
{
    return _labelX;
}

auto chart_base::y_label_count() const -> usize
{
    return _labelY;
}

auto chart_base::position_in_xaxis(f32 value, rect_f const& bounds) const -> f32
{
    f32 const range {XAxis->Max - XAxis->Min};
    if (range == 0.0f) { return bounds.left(); }
    f32 const norm {(value - XAxis->Min) / range};
    return bounds.left() + (norm * bounds.width());
}

auto chart_base::position_in_yaxis(f32 value, rect_f const& bounds) const -> f32
{
    f32 const range {YAxis->Max - YAxis->Min};
    if (range == 0.0f) { return bounds.bottom(); }
    f32 const norm {(value - YAxis->Min) / range};
    return bounds.bottom() - (norm * bounds.height());
}

////////////////////////////////////////////////////////////

void draw_chart_marker(gfx::canvas& canvas, point_f p, marker const& marker, color markerColor, f32 markerSize, color outlineColor, f32 outlineWidth)
{
    if (marker.Type == marker::type::None) { return; }

    auto const drawShape {[&] {
        canvas.begin_path();
        switch (marker.Type) {
        case marker::type::Disc:     canvas.circle(p, markerSize); break;
        case marker::type::Square:   canvas.rect({p.X - markerSize, p.Y - markerSize, markerSize * 2.0f, markerSize * 2.0f}); break;
        case marker::type::Triangle: canvas.triangle({p.X, p.Y - markerSize}, {p.X + markerSize, p.Y + markerSize}, {p.X - markerSize, p.Y + markerSize}); break;
        case marker::type::None:     break;
        }
    }};

    if (marker.Filled) {
        drawShape();
        canvas.set_fill_style(markerColor);
        canvas.set_stroke_style(outlineColor);
        canvas.set_stroke_width(outlineWidth);
        canvas.fill();
        canvas.stroke();
    } else {
        drawShape();
        canvas.set_stroke_style(outlineColor);
        canvas.set_stroke_width(markerSize);
        canvas.stroke();
        drawShape();
        canvas.set_stroke_style(markerColor);
        canvas.set_stroke_width(markerSize - outlineWidth);
        canvas.stroke();
    }
}
}
