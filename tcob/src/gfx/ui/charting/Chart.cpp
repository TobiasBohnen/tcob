// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/charting/Chart.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <utility>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/charting/Charting.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui::charts {

////////////////////////////////////////////////////////////

static void draw_x_labels(widget_painter& painter, chart_style const& style, u32 fontSize, rect_f const& labelArea, std::vector<string> const& labels, bool slots)
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
        painter.draw_text(style.XAxisText, labelRect, painter.format_text(style.XAxisText, labelRect.Size, labels[i], fontSize));
    }
}

static void draw_y_labels(widget_painter& painter, chart_style const& style, u32 fontSize, rect_f const& labelArea, std::vector<string> const& labels)
{
    if (!style.YAxisText.Font || labels.empty()) { return; }
    usize const count {labels.size()};
    if (count <= 1) { return; }
    f32 const yStep {labelArea.height() / static_cast<f32>(count - 1)};
    for (usize i {0}; i < count; ++i) {
        f32 const    cy {labelArea.top() + (static_cast<f32>(i) * yStep)};
        rect_f const labelRect {labelArea.left(), cy - (yStep / 2.0f), labelArea.width(), yStep};
        painter.draw_text(style.YAxisText, labelRect, painter.format_text(style.YAxisText, labelRect.Size, labels[count - 1 - i], fontSize));
    }
}

static auto get_color(auto&& vec, usize i) -> color
{
    return vec[i % vec.size()];
}

////////////////////////////////////////////////////////////

void line_chart::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    grid_chart_style::Transition(target, from, to, step);

    target.LineSize    = helper::lerp(from.LineSize, to.LineSize, step);
    target.SmoothLines = helper::lerp(from.SmoothLines, to.SmoothLines, step);

    target.MarkerType   = helper::lerp(from.MarkerType, to.MarkerType, step);
    target.MarkerSize   = helper::lerp(from.MarkerSize, to.MarkerSize, step);
    target.MarkerColor  = helper::lerp(from.MarkerColor, to.MarkerColor, step);
    target.MarkerFilled = helper::lerp(from.MarkerFilled, to.MarkerFilled, step);
}

line_chart::line_chart(init const& wi)
    : grid_chart {wi}
{
    Class("line_chart");
}

void line_chart::on_draw_chart(widget_painter& painter, std::vector<string> const& xLabels, std::vector<string> const& yLabels)
{
    rect_f const         rect {draw_background(_style, painter)};
    scoped_scissor const guard {painter, this};
    auto&                canvas {painter.canvas()};

    f32 const    tickSize {_style.TickSize.calc(rect.width())};
    f32 const    xLabelHeight {xLabels.empty() ? 0.0f : _style.XLabelHeight.calc(rect.height())};
    f32 const    yLabelWidth {yLabels.empty() ? 0.0f : _style.YLabelWidth.calc(rect.width())};
    f32 const    leftPad {std::max(xLabelHeight + tickSize, yLabelWidth + tickSize)};
    f32 const    topPad {std::max((xLabelHeight + tickSize) / 2.0f, (yLabelWidth + tickSize) / 2.0f)};
    rect_f const chartRect {rect.left() + leftPad, rect.top() + topPad,
                            rect.width() - (leftPad * 2.0f), rect.height() - (topPad * 3.0f)};

    draw_grid(canvas, _style, tickSize, chartRect);

    f32 const   lineWidth {_style.LineSize.calc(rect.width())};
    f32 const   outlineWidth {_style.OutlineSize.calc(rect.width())};
    usize const xCount {x_label_count() > 0 ? x_label_count() : max_x()};
    auto const  cxStep {chartRect.width() / (xCount - 1)};
    for (usize i {0}; i < Dataset->size(); ++i) {
        auto const& s {Dataset[i]};
        usize const len {s.Value.size()};
        if (len < 2) { continue; }

        std::vector<point_f> points;
        for (usize j {0}; j < len; ++j) {
            points.emplace_back(chartRect.left() + (cxStep * static_cast<f32>(j)),
                                position_in_yaxis(s.Value[j], chartRect));
        }
        if (points.size() == 1) { continue; }

        canvas.begin_path();
        canvas.move_to(points[0]);
        if (_style.SmoothLines) {
            for (usize j {0}; j + 1 < points.size(); ++j) {
                auto const& p0 {(j == 0) ? points[j] : points[j - 1]};
                auto const& p1 {points[j]};
                auto const& p2 {points[j + 1]};
                auto const& p3 {(j + 2 < points.size()) ? points[j + 2] : points[j + 1]};

                point_f const c1 {p1.X + ((p2.X - p0.X) * 0.5f / 3.0f),
                                  p1.Y + ((p2.Y - p0.Y) * 0.5f / 3.0f)};
                point_f const c2 {p2.X - ((p3.X - p1.X) * 0.5f / 3.0f),
                                  p2.Y - ((p3.Y - p1.Y) * 0.5f / 3.0f)};

                canvas.cubic_bezier_to(c1, c2, p2);
            }
        } else {
            for (u32 j {1}; j < points.size(); ++j) {
                canvas.line_to(points[j]);
            }
        }

        canvas.set_stroke_style(get_color(_style.OutlineColors, i));
        canvas.set_stroke_width(lineWidth + outlineWidth);
        canvas.stroke();
        canvas.set_stroke_style(get_color(_style.Colors, i));
        canvas.set_stroke_width(lineWidth);
        canvas.stroke();

        if (_style.MarkerType != marker_type::None) {
            f32 const markerSize {_style.MarkerSize.calc(chartRect.width())};
            for (auto const& p : points) {
                auto const drawShape {[&] {
                    canvas.begin_path();
                    switch (_style.MarkerType) {
                    case marker_type::Disc:     canvas.circle(p, markerSize); break;
                    case marker_type::Square:   canvas.rect({p.X - markerSize, p.Y - markerSize, markerSize * 2.0f, markerSize * 2.0f}); break;
                    case marker_type::Triangle: canvas.triangle({p.X, p.Y - markerSize}, {p.X + markerSize, p.Y + markerSize}, {p.X - markerSize, p.Y + markerSize}); break;
                    case marker_type::None:     break;
                    }
                }};

                if (_style.MarkerFilled) {
                    drawShape();
                    canvas.set_fill_style(_style.MarkerColor);
                    canvas.set_stroke_style(get_color(_style.OutlineColors, i));
                    canvas.set_stroke_width(outlineWidth);
                    canvas.fill();
                    canvas.stroke();
                } else {
                    drawShape();
                    canvas.set_stroke_style(get_color(_style.OutlineColors, i));
                    canvas.set_stroke_width(markerSize);
                    canvas.stroke();
                    drawShape();
                    canvas.set_stroke_style(_style.MarkerColor);
                    canvas.set_stroke_width(markerSize - outlineWidth);
                    canvas.stroke();
                }
            }
        }
    }

    rect_f const xLabelArea {chartRect.left(), rect.bottom() - xLabelHeight, chartRect.width(), xLabelHeight};
    u32 const    xFontSize {_style.XAxisText.calc_font_size(xLabelArea.height())};
    draw_x_labels(painter, _style, xFontSize, xLabelArea, xLabels, false);

    rect_f const yLabelArea {rect.left(), chartRect.top(), yLabelWidth, chartRect.height()};
    u32 const    yFontSize {_style.YAxisText.calc_font_size(yLabelArea.height() / y_label_count())};
    draw_y_labels(painter, _style, yFontSize, yLabelArea, yLabels);
}

auto line_chart::calc_grid_lines() const -> std::pair<i32, i32>
{
    isize verticalGridLines {0};
    switch (_style.VerticalGridLines) {
    case grid_line_amount::None:   break;
    case grid_line_amount::Few:    verticalGridLines = (x_label_count() / 2); break;
    case grid_line_amount::Normal: verticalGridLines = x_label_count(); break;
    case grid_line_amount::Many:   verticalGridLines = (x_label_count() * 2) - 1; break;
    }
    isize horizontalGridLines {0};
    switch (_style.HorizontalGridLines) {
    case grid_line_amount::None:   break;
    case grid_line_amount::Few:    horizontalGridLines = (y_label_count() / 2); break;
    case grid_line_amount::Normal: horizontalGridLines = y_label_count(); break;
    case grid_line_amount::Many:   horizontalGridLines = (y_label_count() * 2) - 1; break;
    }
    return {std::max(2, static_cast<i32>(horizontalGridLines)), std::max(2, static_cast<i32>(verticalGridLines))};
}

////////////////////////////////////////////////////////////

void bar_chart::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    grid_chart_style::Transition(target, from, to, step);

    target.BarSize   = helper::lerp(from.BarSize, to.BarSize, step);
    target.BarRadius = helper::lerp(from.BarRadius, to.BarRadius, step);
    target.StackBars = helper::lerp(from.StackBars, to.StackBars, step);
}

bar_chart::bar_chart(init const& wi)
    : grid_chart {wi}
{
    Class("bar_chart");
}

void bar_chart::on_draw_chart(widget_painter& painter, std::vector<string> const& xLabels, std::vector<string> const& yLabels)
{
    rect_f const         rect {draw_background(_style, painter)};
    scoped_scissor const guard {painter, this};
    auto&                canvas {painter.canvas()};

    f32 const    tickSize {_style.TickSize.calc(rect.width())};
    usize const  xLabelCount {x_label_count()};
    f32 const    xLabelHeight {xLabelCount == 0 ? 0.0f : _style.XLabelHeight.calc(rect.height())};
    f32 const    yLabelWidth {y_label_count() == 0 ? 0.0f : _style.YLabelWidth.calc(rect.width())};
    f32 const    leftPad {yLabelWidth + tickSize};
    f32 const    topPad {std::max((xLabelHeight + tickSize) / 2.0f, (yLabelWidth + tickSize) / 2.0f)};
    rect_f const chartRect {rect.left() + leftPad, rect.top() + topPad,
                            rect.width() - leftPad, rect.height() - (topPad * 2.5f)};

    draw_grid(canvas, _style, tickSize, chartRect);

    usize const barCount {Dataset->size()};
    usize const groupCount {xLabelCount == 0 ? barCount : xLabelCount};
    f32 const   columnWidth {chartRect.width() / static_cast<f32>(groupCount)};
    f32 const   barWidth {_style.BarSize.calc(columnWidth)};
    f32 const   barRadius {_style.BarRadius.calc(barWidth)};
    f32 const   outlineWidth {_style.OutlineSize.calc(chartRect.width())};

    if (_style.StackBars) {
        for (usize j {0}; j < groupCount; ++j) {
            f32 yOffset {0.0f};
            for (usize i {0}; i < barCount; ++i) {
                auto const& s {Dataset[i]};
                if (j >= s.Value.size()) { continue; }

                canvas.set_fill_style(get_color(_style.Colors, i));

                f32 const     yAbs {position_in_yaxis(s.Value[j], chartRect)};
                f32 const     barHeight {chartRect.bottom() - yAbs};
                point_f const pos {chartRect.left() + (columnWidth * static_cast<f32>(j)) + ((columnWidth - barWidth) / 2.0f),
                                   yAbs - yOffset};

                canvas.begin_path();
                canvas.rounded_rect({pos, {barWidth, barHeight}}, barRadius);
                canvas.fill();
                canvas.set_stroke_width(outlineWidth);
                canvas.set_stroke_style(get_color(_style.OutlineColors, i));
                canvas.stroke();

                yOffset += barHeight;
            }
        }

    } else {
        for (usize i {0}; i < barCount; ++i) {
            canvas.set_fill_style(get_color(_style.Colors, i));

            auto const& s {Dataset[i]};
            usize const valueCount {s.Value.size()};
            f32 const   xOffset {chartRect.left() + (barWidth / barCount * i) + ((columnWidth - barWidth) / 2.0f)};
            for (usize j {0}; j < valueCount; ++j) {
                point_f const pos {xOffset + (columnWidth * static_cast<f32>(j)),
                                   position_in_yaxis(s.Value[j], chartRect)};

                canvas.begin_path();
                canvas.rounded_rect({pos, {barWidth / static_cast<f32>(barCount), chartRect.bottom() - pos.Y}}, barRadius);
                canvas.fill();
                canvas.set_stroke_width(outlineWidth);
                canvas.set_stroke_style(get_color(_style.OutlineColors, i));
                canvas.stroke();
            }
        }
    }

    rect_f const xLabelArea {chartRect.left(), rect.bottom() - xLabelHeight, chartRect.width(), xLabelHeight};
    u32 const    xFontSize {_style.XAxisText.calc_font_size(xLabelArea.height())};
    draw_x_labels(painter, _style, xFontSize, xLabelArea, xLabels, true);

    rect_f const yLabelArea {rect.left(), chartRect.top(), yLabelWidth, chartRect.height()};
    u32 const    yFontSize {_style.YAxisText.calc_font_size(yLabelArea.height() / y_label_count())};
    draw_y_labels(painter, _style, yFontSize, yLabelArea, yLabels);
}

auto bar_chart::calc_grid_lines() const -> std::pair<i32, i32>
{
    isize verticalGridLines {0};
    switch (_style.VerticalGridLines) {
    case grid_line_amount::None:   break;
    case grid_line_amount::Few:    verticalGridLines = (x_label_count() / 2) + 1; break;
    case grid_line_amount::Normal: verticalGridLines = x_label_count() + 1; break;
    case grid_line_amount::Many:   verticalGridLines = (x_label_count() * 2) + 1; break;
    }
    isize horizontalGridLines {0};
    switch (_style.HorizontalGridLines) {
    case grid_line_amount::None:   break;
    case grid_line_amount::Few:    horizontalGridLines = (y_label_count() / 2); break;
    case grid_line_amount::Normal: horizontalGridLines = y_label_count(); break;
    case grid_line_amount::Many:   horizontalGridLines = (y_label_count() * 2) - 1; break;
    }
    return {std::max(2, static_cast<i32>(horizontalGridLines)), std::max(2, static_cast<i32>(verticalGridLines))};
}

////////////////////////////////////////////////////////////

void marimekko_chart::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    chart_style::Transition(target, from, to, step);

    target.BarSize   = helper::lerp(from.BarSize, to.BarSize, step);
    target.BarRadius = helper::lerp(from.BarRadius, to.BarRadius, step);
}

marimekko_chart::marimekko_chart(init const& wi)
    : chart {wi}
{
    Class("marimekko_chart");
}

void marimekko_chart::on_draw_chart(widget_painter& painter, std::vector<string> const& xLabels, std::vector<string> const& /* yLabels */)
{
    rect_f const         rect {draw_background(_style, painter)};
    scoped_scissor const guard {painter, this};
    auto&                canvas {painter.canvas()};

    usize const seriesCount {Dataset->size()};
    if (seriesCount == 0) { return; }

    usize const valueCount {Dataset->front().Value.size()};
    if (valueCount == 0) { return; }

    std::vector<f32> columnTotals(valueCount, 0.0f);
    for (usize j {0}; j < valueCount; ++j) {
        for (usize i {0}; i < seriesCount; ++i) {
            if (j < Dataset[i].Value.size()) {
                columnTotals[j] += Dataset[i].Value[j];
            }
        }
    }

    f32 const total {std::accumulate(columnTotals.begin(), columnTotals.end(), 0.0f)};
    if (total == 0.0f) { return; }

    usize const  xLabelCount {x_label_count()};
    f32 const    xLabelHeight {xLabelCount == 0 ? 0.0f : _style.XLabelHeight.calc(rect.height())};
    rect_f const chartRect {rect.left(), rect.top(), rect.width(), rect.height() - xLabelHeight};

    f32 const outlineWidth {_style.OutlineSize.calc(chartRect.width())};
    f32       xCursor {chartRect.left()};

    for (usize j {0}; j < valueCount; ++j) {
        f32 const  columnFraction {columnTotals[j] / total};
        f32 const  columnWidth {chartRect.width() * columnFraction};
        auto const barWidth {_style.BarSize.Width.calc(columnWidth)};
        auto const barRadius {_style.BarRadius.calc(barWidth)};

        f32       yOffset {0.0f};
        f32 const baseline {chartRect.bottom()};

        for (usize i {0}; i < seriesCount; ++i) {
            auto const& s {Dataset[i]};
            if (j >= s.Value.size()) { continue; }

            f32 const value {s.Value[j]};
            if (columnTotals[j] == 0.0f) { continue; }

            f32 const     height {chartRect.height() * (value / columnTotals[j])};
            auto const    barHeight {_style.BarSize.Height.calc(height)};
            point_f const pos {xCursor + ((columnWidth - barWidth) / 2.0f),
                               baseline - (yOffset + height) + ((height - barHeight) / 2.0f)};

            canvas.set_fill_style(get_color(_style.Colors, i));
            canvas.begin_path();
            canvas.rounded_rect({pos, {barWidth, barHeight}}, barRadius);
            canvas.fill();
            canvas.set_stroke_width(outlineWidth);
            canvas.set_stroke_style(get_color(_style.OutlineColors, i));
            canvas.stroke();

            yOffset += height;
        }

        if (_style.XAxisText.Font && j < xLabels.size()) {
            rect_f const labelRect {xCursor, rect.bottom() - xLabelHeight, columnWidth, xLabelHeight};
            painter.draw_text(_style.XAxisText, labelRect, xLabels[j]);
        }

        xCursor += columnWidth;
    }
}

////////////////////////////////////////////////////////////

void pie_chart::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    chart_style::Transition(target, from, to, step);

    target.InnerRadius = helper::lerp(from.InnerRadius, to.InnerRadius, step);
    target.PadAngle    = helper::lerp(from.PadAngle, to.PadAngle, step);
}

pie_chart::pie_chart(init const& wi)
    : chart {wi}
{
    Class("pie_chart");
}

void pie_chart::on_draw_chart(widget_painter& painter, std::vector<string> const& /* xLabels */, std::vector<string> const& /* yLabels */)
{
    rect_f const         rect {draw_background(_style, painter)};
    scoped_scissor const guard {painter, this};
    auto&                canvas {painter.canvas()};

    f64 total {0.0};
    for (auto const& s : *Dataset) { total += s.Value; }
    if (total == 0.0) { return; }

    f32 const  outlineWidth {_style.OutlineSize.calc(rect.width())};
    auto const center {rect.center()};
    f32 const  radius {(std::min(rect.width(), rect.height()) / 2.0f) - (outlineWidth / 2.0f)};
    f32 const  innerRadius {_style.InnerRadius.calc(radius)};
    bool const isDonut {innerRadius > 0.0f};
    f64 const  padAngle {radian_d {_style.PadAngle}.Value};

    f64 const outerHalfPad {padAngle / 2.0};
    f64 const innerHalfPad {isDonut ? std::atan2(std::sin(outerHalfPad) * radius, innerRadius)
                                    : outerHalfPad};

    f64 angle {0.0};
    for (usize i {0}; i < Dataset->size(); ++i) {
        auto const& s {Dataset[i]};
        f64 const   fraction {s.Value / total};
        f64 const   fullSweep {fraction * TAU};
        f64 const   outerSweep {std::max(0.0, fullSweep - padAngle)};
        f64 const   innerSweep {std::max(0.0, fullSweep - innerHalfPad * 2.0)};

        if (outerSweep <= 0.0) {
            angle += fullSweep;
            continue;
        }

        f64 const outerStart {angle + outerHalfPad};
        f64 const innerStart {angle + innerHalfPad};

        canvas.begin_path();
        if (isDonut) {
            canvas.arc(center, radius,
                       radian_d {outerStart}, radian_d {outerStart + outerSweep},
                       gfx::winding::CW);
            canvas.arc(center, innerRadius,
                       radian_d {innerStart + innerSweep}, radian_d {innerStart},
                       gfx::winding::CCW);
        } else {
            canvas.move_to(center);
            canvas.arc(center, radius,
                       radian_d {outerStart}, radian_d {outerStart + outerSweep},
                       gfx::winding::CW);
        }
        canvas.close_path();

        canvas.set_fill_style(get_color(_style.Colors, i));
        canvas.fill();
        canvas.set_stroke_width(outlineWidth);
        canvas.set_stroke_style(get_color(_style.OutlineColors, i));
        canvas.stroke();

        angle += fullSweep;
    }
}

////////////////////////////////////////////////////////////

void scatter_chart::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    chart_style::Transition(target, from, to, step);

    target.PointSize = helper::lerp(from.PointSize, to.PointSize, step);
}

scatter_chart::scatter_chart(init const& wi)
    : grid_chart {wi}
{
    Class("scatter_chart");

    XAxis.Changed.connect([&](axis const& axis) {
        if (_ignoreAxisChange) { return; }

        _initialX.Min       = axis.Min;
        _initialX.Max       = axis.Max;
        _initialX.SmallStep = axis.SmallStep;
        _initialX.LargeStep = axis.LargeStep;
    });
    YAxis.Changed.connect([&](axis const& axis) {
        if (_ignoreAxisChange) { return; }

        _initialY.Min       = axis.Min;
        _initialY.Max       = axis.Max;
        _initialY.SmallStep = axis.SmallStep;
        _initialY.LargeStep = axis.LargeStep;
    });
}

void scatter_chart::on_draw_chart(widget_painter& painter, std::vector<string> const& xLabels, std::vector<string> const& yLabels)
{
    rect_f const         rect {draw_background(_style, painter)};
    scoped_scissor const guard {painter, this};
    auto&                canvas {painter.canvas()};

    f32 const    tickSize {_style.TickSize.calc(rect.width())};
    f32 const    pointSize {_style.PointSize.calc(rect.width())};
    f32 const    xLabelHeight {xLabels.empty() ? 0.0f : _style.XLabelHeight.calc(rect.height())};
    f32 const    yLabelWidth {yLabels.empty() ? 0.0f : _style.YLabelWidth.calc(rect.width())};
    f32 const    leftPad {std::max(yLabelWidth + tickSize + pointSize, xLabelHeight + tickSize)};
    f32 const    topPad {std::max((xLabelHeight + tickSize + pointSize) / 2.0f, (yLabelWidth + tickSize + pointSize) / 2.0f)};
    rect_f const chartRect {rect.left() + leftPad, rect.top() + topPad,
                            rect.width() - (leftPad * 2.0f), rect.height() - (topPad * 2.5f)};

    draw_grid(canvas, _style, tickSize, chartRect);

    f32 const outlineWidth {_style.OutlineSize.calc(chartRect.width())};

    for (usize i {0}; i < Dataset->size(); ++i) {
        auto const& s {Dataset[i]};
        canvas.set_fill_style(get_color(_style.Colors, i));
        canvas.set_stroke_style(get_color(_style.OutlineColors, i));
        canvas.set_stroke_width(outlineWidth);

        canvas.begin_path();
        for (auto const& pt : s.Value) {
            point_f const p {
                position_in_xaxis(pt.X, chartRect),
                position_in_yaxis(pt.Y, chartRect)};
            if (chartRect.contains(p, true)) {
                canvas.circle(p, pointSize);
            }
        }
        canvas.fill();
        canvas.stroke();
    }

    rect_f const xLabelArea {chartRect.left(), rect.bottom() - xLabelHeight, chartRect.width(), xLabelHeight};
    u32 const    xFontSize {_style.XAxisText.calc_font_size(xLabelArea.height())};
    draw_x_labels(painter, _style, xFontSize, xLabelArea, xLabels, false);

    rect_f const yLabelArea {rect.left(), chartRect.top(), yLabelWidth, chartRect.height()};
    u32 const    yFontSize {_style.YAxisText.calc_font_size(yLabelArea.height() / y_label_count())};
    draw_y_labels(painter, _style, yFontSize, yLabelArea, yLabels);
}

void scatter_chart::on_mouse_drag(input::mouse::motion_event const& ev)
{
    _ignoreAxisChange = true;

    f32 const xRange {XAxis->Max - XAxis->Min};
    f32 const yRange {YAxis->Max - YAxis->Min};

    _dragAccum.X += (static_cast<f32>(ev.RelativeMotion.X) / grid_rect().width()) * xRange;
    _dragAccum.Y += (static_cast<f32>(ev.RelativeMotion.Y) / grid_rect().height()) * yRange;

    XAxis.mutate([&](axis& a) {
        f32 const snap {helper::round_to_multiple(_dragAccum.X, a.SmallStep)};
        if (snap != 0.0f) {
            a.Min -= snap;
            a.Max -= snap;
            _dragAccum.X -= snap;
        }
    });
    YAxis.mutate([&](axis& a) {
        f32 const snap {helper::round_to_multiple(_dragAccum.Y, a.SmallStep)};
        if (snap != 0.0f) {
            a.Min += snap;
            a.Max += snap;
            _dragAccum.Y -= snap;
        }
    });

    _ignoreAxisChange = false;
}

void scatter_chart::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    _ignoreAxisChange = true;

    f32 const xRange {XAxis->Max - XAxis->Min};
    f32 const yRange {YAxis->Max - YAxis->Min};
    f32 const xCenter {(XAxis->Min + XAxis->Max) / 2.0f};
    f32 const yCenter {(YAxis->Min + YAxis->Max) / 2.0f};
    f32 const factor {1.0f - (ev.Scroll.Y * 0.1f)};

    XAxis.mutate([&](axis& a) {
        a.Min = xCenter - (xRange * factor / 2.0f);
        a.Max = xCenter + (xRange * factor / 2.0f);
        a.LargeStep *= factor;
        a.SmallStep *= factor;
    });
    YAxis.mutate([&](axis& a) {
        a.Min = yCenter - (yRange * factor / 2.0f);
        a.Max = yCenter + (yRange * factor / 2.0f);
        a.LargeStep *= factor;
        a.SmallStep *= factor;
    });
    _dragAccum = {};

    _ignoreAxisChange = false;
}

void scatter_chart::on_double_click()
{
    _ignoreAxisChange = true;
    XAxis.mutate([&](axis& a) { a.Min = _initialX.Min; a.Max = _initialX.Max; a.SmallStep = _initialX.SmallStep; a.LargeStep = _initialX.LargeStep; });
    YAxis.mutate([&](axis& a) { a.Min = _initialY.Min; a.Max = _initialY.Max; a.SmallStep = _initialY.SmallStep; a.LargeStep = _initialY.LargeStep; });
    _dragAccum        = {};
    _ignoreAxisChange = false;
}

auto scatter_chart::calc_grid_lines() const -> std::pair<i32, i32>
{
    isize verticalGridLines {0};
    switch (_style.VerticalGridLines) {
    case grid_line_amount::None:   break;
    case grid_line_amount::Few:    verticalGridLines = (x_label_count() / 2); break;
    case grid_line_amount::Normal: verticalGridLines = x_label_count(); break;
    case grid_line_amount::Many:   verticalGridLines = (x_label_count() * 2) - 1; break;
    }
    isize horizontalGridLines {0};
    switch (_style.HorizontalGridLines) {
    case grid_line_amount::None:   break;
    case grid_line_amount::Few:    horizontalGridLines = (y_label_count() / 2); break;
    case grid_line_amount::Normal: horizontalGridLines = y_label_count(); break;
    case grid_line_amount::Many:   horizontalGridLines = (y_label_count() * 2) - 1; break;
    }
    return {std::max(2, static_cast<i32>(horizontalGridLines)), std::max(2, static_cast<i32>(verticalGridLines))};
}

////////////////////////////////////////////////////////////

void radar_chart::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    chart_style::Transition(target, from, to, step);
    target.FillAreaAlpha = helper::lerp(from.FillAreaAlpha, to.FillAreaAlpha, step);

    target.LineSize     = helper::lerp(from.LineSize, to.LineSize, step);
    target.GridLineSize = helper::lerp(from.GridLineSize, to.GridLineSize, step);
    target.GridColor    = helper::lerp(from.GridColor, to.GridColor, step);
}

radar_chart::radar_chart(init const& wi)
    : chart {wi}
{
    Class("radar_chart");
}

void radar_chart::on_draw_chart(widget_painter& painter, std::vector<string> const& xLabels, std::vector<string> const& /* yLabels */)
{
    rect_f const         rect {draw_background(_style, painter)};
    scoped_scissor const guard {painter, this};
    auto&                canvas {painter.canvas()};

    usize axisCount {0};
    for (auto const& s : *Dataset) { axisCount = std::max(axisCount, s.Value.size()); }
    if (axisCount == 0) { return; }

    f32 const     labelHeight {xLabels.empty() ? 0.0f : _style.XLabelHeight.calc(rect.height())};
    point_f const center {rect.center()};
    f32 const     radius {(std::min(rect.width(), rect.height()) / 2.0f) - labelHeight};

    if (_style.GridLines != grid_line_amount::None) {
        canvas.set_stroke_style(_style.GridColor);
        canvas.set_stroke_width(_style.GridLineSize.calc(radius));
        for (usize i {0}; i < axisCount; ++i) {
            canvas.begin_path();
            canvas.move_to(center);
            f32 const     angle {(static_cast<f32>(i) / axisCount) * TAU_F};
            point_f const pos {center.X + (std::cos(angle) * radius),
                               center.Y + (std::sin(angle) * radius)};
            canvas.line_to(pos);
            canvas.stroke();
        }

        usize ringCount {0};
        switch (_style.GridLines) {
        case grid_line_amount::None:   break;
        case grid_line_amount::Few:    ringCount = 3; break;
        case grid_line_amount::Normal: ringCount = 6; break;
        case grid_line_amount::Many:   ringCount = 12; break;
        }
        for (usize r {1}; r <= ringCount; ++r) {
            f32 const ringRadius {(static_cast<f32>(r) / ringCount) * radius};
            canvas.begin_path();
            for (usize i {0}; i < axisCount; ++i) {
                f32 const     angle {(static_cast<f32>(i) / axisCount) * TAU_F};
                point_f const pos {center.X + (std::cos(angle) * ringRadius),
                                   center.Y + (std::sin(angle) * ringRadius)};
                if (i == 0) {
                    canvas.move_to(pos);
                } else {
                    canvas.line_to(pos);
                }
            }
            canvas.close_path();
            canvas.stroke();
        }
    }

    f32 const lineWidth {_style.LineSize.calc(radius)};
    f32 const outlineWidth {_style.OutlineSize.calc(radius)};

    for (usize i {0}; i < Dataset->size(); ++i) {
        auto const& s {Dataset[i]};
        if (s.Value.empty()) { continue; }
        std::vector<point_f> points;
        for (usize j {0}; j < axisCount; ++j) {
            f32 const value {(j < s.Value.size()) ? s.Value[j] : 0.0f};
            f32 const normalized {(value - YAxis->Min) / (YAxis->Max - YAxis->Min)};
            f32 const angle {(static_cast<f32>(j) / axisCount) * TAU_F};
            f32 const r {normalized * radius};

            points.emplace_back(center.X + (std::cos(angle) * r), center.Y + (std::sin(angle) * r));
        }

        canvas.begin_path();
        canvas.move_to(points[0]);
        for (usize j {1}; j < points.size(); ++j) { canvas.line_to(points[j]); }
        canvas.close_path();

        color const c {get_color(_style.Colors, i)};
        if (_style.FillAreaAlpha > 0) {
            canvas.set_fill_style({c.R, c.G, c.B, _style.FillAreaAlpha});
            canvas.fill();

            canvas.set_stroke_style(get_color(_style.OutlineColors, i));
            canvas.set_stroke_width(outlineWidth);
            canvas.stroke();
        } else {
            canvas.set_stroke_style(get_color(_style.OutlineColors, i));
            canvas.set_stroke_width(lineWidth + (outlineWidth * 2));
            canvas.stroke();

            canvas.set_stroke_style(c);
            canvas.set_stroke_width(lineWidth);
            canvas.stroke();
        }
    }

    if (_style.XAxisText.Font && !xLabels.empty()) {
        for (usize i {0}; i < std::min(axisCount, xLabels.size()); ++i) {
            f32 const    angle {(static_cast<f32>(i) / axisCount) * TAU_F};
            f32 const    x {center.X + (std::cos(angle) * (radius + (labelHeight / 2.0f)))};
            f32 const    y {center.Y + (std::sin(angle) * (radius + (labelHeight / 2.0f)))};
            rect_f const labelRect {x - labelHeight, y - (labelHeight / 2.0f), labelHeight * 2.0f, labelHeight};
            painter.draw_text(_style.XAxisText, labelRect, xLabels[i]);
        }
    }
}

}
