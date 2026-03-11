// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/charting/Charting.hpp"

#include <cmath>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui::charts {

////////////////////////////////////////////////////////////

void marker_element::lerp(marker_element const& from, marker_element const& to, f64 step)
{
    Type   = helper::lerp(from.Type, to.Type, step);
    Size   = helper::lerp(from.Size, to.Size, step);
    Color  = helper::lerp(from.Color, to.Color, step);
    Filled = helper::lerp(from.Filled, to.Filled, step);
}

////////////////////////////////////////////////////////////

void chart_style::Transition(chart_style& target, chart_style const& from, chart_style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.XAxisText.lerp(from.XAxisText, to.XAxisText, step);
    target.XLabelHeight = helper::lerp(from.XLabelHeight, to.XLabelHeight, step);
    target.YAxisText.lerp(from.YAxisText, to.YAxisText, step);
    target.YLabelWidth = helper::lerp(from.YLabelWidth, to.YLabelWidth, step);

    target.OutlineSize = helper::lerp(from.OutlineSize, to.OutlineSize, step);
}

////////////////////////////////////////////////////////////

void grid_chart_style::Transition(grid_chart_style& target, grid_chart_style const& from, grid_chart_style const& to, f64 step)
{
    chart_style::Transition(target, from, to, step);

    target.GridLineSize = helper::lerp(from.GridLineSize, to.GridLineSize, step);
    target.GridColor    = helper::lerp(from.GridColor, to.GridColor, step);
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

}
