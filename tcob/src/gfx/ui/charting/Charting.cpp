// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/charting/Charting.hpp"

#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui::charts {

void chart_style::Transition(chart_style& target, chart_style const& from, chart_style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.XAxisText.lerp(from.XAxisText, to.XAxisText, step);
    target.XLabelHeight = helper::lerp(from.XLabelHeight, to.XLabelHeight, step);
    target.YAxisText.lerp(from.YAxisText, to.YAxisText, step);
    target.YLabelWidth = helper::lerp(from.YLabelWidth, to.YLabelWidth, step);

    target.Colors.clear();
    target.Colors.resize(from.Colors.size());
    for (usize i {0}; i < from.Colors.size(); ++i) {
        if (i >= to.Colors.size()) {
            target.Colors[i] = from.Colors[i];
        } else {
            target.Colors[i] = helper::lerp(from.Colors[i], to.Colors[i], step);
        }
    }

    target.OutlineColor = helper::lerp(from.OutlineColor, to.OutlineColor, step);
    target.OutlineSize  = helper::lerp(from.OutlineSize, to.OutlineSize, step);
}

void grid_chart_style::Transition(grid_chart_style& target, grid_chart_style const& from, grid_chart_style const& to, f64 step)
{
    chart_style::Transition(target, from, to, step);

    target.GridLineSize = helper::lerp(from.GridLineSize, to.GridLineSize, step);
    target.GridColor    = helper::lerp(from.GridColor, to.GridColor, step);
}

chart_base::chart_base(init const& wi)
    : widget {wi}
{
    XAxis.Changed.connect([&](axis const& axis) {
        _labelX = axis.CustomLabels.empty() ? axis.LabelCount : axis.CustomLabels.size();
        queue_redraw();
    });
    YAxis.Changed.connect([&](axis const& axis) {
        _labelY = axis.CustomLabels.empty() ? axis.LabelCount : axis.CustomLabels.size();
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

void chart_base::draw_x_labels(widget_painter& painter, chart_style const& style, rect_f const& labelArea, std::vector<string> const& labels, bool slots) const
{
    if (!style.XAxisText.Font || labels.empty()) { return; }
    usize const count {labels.size()};
    f32 const   xStep {slots ? labelArea.width() / static_cast<f32>(count)
                             : labelArea.width() / static_cast<f32>(count - 1)};
    if (!slots && count <= 1) { return; }
    for (usize i {0}; i < count; ++i) {
        f32 const    cx {labelArea.left() + (static_cast<f32>(i) * xStep)};
        rect_f const labelRect {slots ? rect_f {cx, labelArea.top(), xStep, labelArea.height()}
                                      : rect_f {cx - (xStep / 2.0f), labelArea.top(), xStep, labelArea.height()}};
        painter.draw_text(style.XAxisText, labelRect, labels[i]);
    }
}

void chart_base::draw_y_labels(widget_painter& painter, chart_style const& style, rect_f const& labelArea, std::vector<string> const& labels) const
{
    if (!style.YAxisText.Font || labels.empty()) { return; }
    usize const count {labels.size()};
    if (count <= 1) { return; }
    f32 const yStep {labelArea.height() / static_cast<f32>(count - 1)};
    for (usize i {0}; i < count; ++i) {
        f32 const    cy {labelArea.top() + (static_cast<f32>(i) * yStep)};
        rect_f const labelRect {labelArea.left(), cy - (yStep / 2.0f), labelArea.width(), yStep};
        painter.draw_text(style.YAxisText, labelRect, labels[count - 1 - i]);
    }
}

}
