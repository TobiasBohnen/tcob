// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/charting/Chart.hpp"

#include <algorithm>
#include <numeric>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/charting/Charting.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui::charts {

////////////////////////////////////////////////////////////

void line_chart::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    grid_chart_style::Transition(target, from, to, step);

    target.LineSize = length::Lerp(from.LineSize, to.LineSize, step);
}

line_chart::line_chart(init const& wi)
    : grid_chart {wi}
{
    Class("line_chart");
    YAxis.Changed.connect([&] { queue_redraw(); });
}

void line_chart::on_draw(widget_painter& painter)
{
    rect_f const        rect {draw_background(_style, painter)};
    scissor_guard const guard {painter, this};

    auto& canvas {painter.canvas()};

    draw_grid(canvas, _style, rect);

    auto const xStep {rect.width() / (max_x() - 1)};

    for (usize i {0}; i < Series->size(); ++i) {
        auto const& s {Series[i]};
        usize const len {s.Values.size()};
        if (len < 2) { continue; }

        std::vector<point_f> points;
        for (usize j {0}; j < len; ++j) {
            f32 const x {rect.left() + (xStep * j)};
            f32 const y {position_in_yaxis(s.Values[j], *YAxis, rect)};
            points.emplace_back(x, y);
        }
        if (points.size() == 1) { continue; }

        canvas.begin_path();
        canvas.move_to(points[0]);
        if (_style.SmoothLines) {
            for (usize i {0}; i + 1 < points.size(); ++i) {
                auto const& p0 {(i == 0) ? points[i] : points[i - 1]};
                auto const& p1 {points[i]};
                auto const& p2 {points[i + 1]};
                auto const& p3 {(i + 2 < points.size()) ? points[i + 2] : points[i + 1]};

                point_f c1 {p1.X + ((p2.X - p0.X) * 0.5f / 3.0f),
                            p1.Y + ((p2.Y - p0.Y) * 0.5f / 3.0f)};
                point_f c2 {p2.X - ((p3.X - p1.X) * 0.5f / 3.0f),
                            p2.Y - ((p3.Y - p1.Y) * 0.5f / 3.0f)};

                canvas.cubic_bezier_to(c1, c2, p2);
            }
        } else {
            for (u32 i {1}; i < points.size(); ++i) {
                canvas.line_to(points[i]);
            }
        }

        canvas.set_stroke_style(colors::Black);
        canvas.set_stroke_width(_style.LineSize.calc(rect.width()) + 1);
        canvas.stroke();
        canvas.set_stroke_style(_style.Colors[i % _style.Colors.size()]);
        canvas.set_stroke_width(_style.LineSize.calc(rect.width()));
        canvas.stroke();
    }
}

auto line_chart::calc_grid_lines() const -> size_i
{
    i32 verticalGridLines {0};
    switch (_style.VerticalGridLines) {
    case grid_line_amount::None:   break;
    case grid_line_amount::Few:    verticalGridLines = (max_x() / 2); break;
    case grid_line_amount::Normal: verticalGridLines = max_x(); break;
    case grid_line_amount::Many:   verticalGridLines = (max_x() * 2) - 1; break;
    }
    i32 horizontalGridLines {0};
    switch (_style.HorizontalGridLines) {
    case grid_line_amount::None:   break;
    case grid_line_amount::Few:    horizontalGridLines = 3; break;
    case grid_line_amount::Normal: horizontalGridLines = 5; break;
    case grid_line_amount::Many:   horizontalGridLines = 10; break;
    }
    return {horizontalGridLines, verticalGridLines};
}

////////////////////////////////////////////////////////////

void bar_chart::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    grid_chart_style::Transition(target, from, to, step);

    target.BarSize   = length::Lerp(from.BarSize, to.BarSize, step);
    target.BarRadius = length::Lerp(from.BarRadius, to.BarRadius, step);
}

bar_chart::bar_chart(init const& wi)
    : grid_chart {wi}
{
    Class("bar_chart");
    YAxis.Changed.connect([&] { queue_redraw(); });
}

void bar_chart::on_draw(widget_painter& painter)
{
    rect_f const        rect {draw_background(_style, painter)};
    scissor_guard const guard {painter, this};

    auto& canvas {painter.canvas()};

    draw_grid(canvas, _style, rect);

    auto const barCount {Series->size()};
    if (barCount == 0) { return; }

    auto const columnWidth {rect.width() / max_x()};
    auto const barWidth {_style.BarSize.calc(columnWidth)};
    auto const barRadius {_style.BarRadius.calc(barWidth)};

    if (_style.StackBars) {
        for (usize j {0}; j < max_x(); ++j) {
            f32 yOffset {0.0f};

            for (usize i {0}; i < barCount; ++i) {
                auto const& s {Series[i]};
                if (j >= s.Values.size()) { continue; }

                canvas.set_fill_style(_style.Colors[i % _style.Colors.size()]);

                f32 const yAbs {position_in_yaxis(s.Values[j], *YAxis, rect)};
                f32 const barHeight {rect.bottom() - yAbs};
                f32 const y {yAbs - yOffset};
                f32 const x {rect.left() + (columnWidth * j) + ((columnWidth - barWidth) / 2)};

                canvas.begin_path();
                canvas.rounded_rect({{x, y}, {barWidth, barHeight}}, barRadius);
                canvas.fill();
                canvas.set_stroke_width(1);
                canvas.set_stroke_style(colors::Black);
                canvas.stroke();

                yOffset += barHeight;
            }
        }

    } else {
        for (usize i {0}; i < barCount; ++i) {
            auto const& s {Series[i]};
            usize const valueCount {s.Values.size()};
            if (valueCount == 0) { return; }

            canvas.set_fill_style(_style.Colors[i % _style.Colors.size()]);

            f32 const xOffset {rect.left() + (barWidth / barCount * i) + ((columnWidth - barWidth) / 2)};
            for (usize j {0}; j < valueCount; ++j) {
                f32 const x {xOffset + (columnWidth * j)};
                f32 const y {position_in_yaxis(s.Values[j], *YAxis, rect)};

                canvas.begin_path();
                canvas.rounded_rect({{x, y}, {barWidth / barCount, rect.bottom() - y}}, barRadius);
                canvas.fill();
                canvas.set_stroke_width(1);
                canvas.set_stroke_style(colors::Black);
                canvas.stroke();
            }
        }
    }
}

auto bar_chart::calc_grid_lines() const -> size_i
{
    i32 verticalGridLines {0};
    switch (_style.VerticalGridLines) {
    case grid_line_amount::None:   break;
    case grid_line_amount::Few:    verticalGridLines = (max_x() / 2) + 1; break;
    case grid_line_amount::Normal: verticalGridLines = max_x() + 1; break;
    case grid_line_amount::Many:   verticalGridLines = (max_x() * 2) + 1; break;
    }
    i32 horizontalGridLines {0};
    switch (_style.HorizontalGridLines) {
    case grid_line_amount::None:   break;
    case grid_line_amount::Few:    horizontalGridLines = 3; break;
    case grid_line_amount::Normal: horizontalGridLines = 5; break;
    case grid_line_amount::Many:   horizontalGridLines = 10; break;
    }
    return {horizontalGridLines, verticalGridLines};
}

////////////////////////////////////////////////////////////

void marimekko_chart::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    chart_style::Transition(target, from, to, step);

    target.BarSize   = dimensions::Lerp(from.BarSize, to.BarSize, step);
    target.BarRadius = length::Lerp(from.BarRadius, to.BarRadius, step);
}

marimekko_chart::marimekko_chart(init const& wi)
    : chart {wi}
{
    Class("marimekko_chart");
}

void marimekko_chart::on_draw(widget_painter& painter)
{
    rect_f const        rect {draw_background(_style, painter)};
    scissor_guard const guard {painter, this};

    auto& canvas {painter.canvas()};

    usize const seriesCount {Series->size()};
    if (seriesCount == 0) { return; }

    usize const valueCount {Series->front().Values.size()};
    if (valueCount == 0) { return; }

    std::vector<f32> columnTotals(valueCount, 0.0f);
    for (usize j {0}; j < valueCount; ++j) {
        for (usize i {0}; i < seriesCount; ++i) {
            if (j < Series[i].Values.size()) {
                columnTotals[j] += Series[i].Values[j];
            }
        }
    }

    f32 const total {std::accumulate(columnTotals.begin(), columnTotals.end(), 0.0f)};
    if (total == 0.0f) { return; }

    f32 xCursor {rect.left()};

    for (usize j {0}; j < valueCount; ++j) {
        f32 const  columnFraction {columnTotals[j] / total};
        f32 const  columnWidth {rect.width() * columnFraction};
        auto const barWidth {_style.BarSize.Width.calc(columnWidth)};
        auto const barRadius {_style.BarRadius.calc(barWidth)};

        f32       yOffset {0.0f};
        f32 const baseline {rect.bottom()};

        for (usize i {0}; i < seriesCount; ++i) {
            auto const& s {Series[i]};
            if (j >= s.Values.size()) { continue; }

            f32 const value {s.Values[j]};
            if (columnTotals[j] == 0.0f) { continue; }

            f32 const  height {rect.height() * (value / columnTotals[j])};
            auto const barHeight {_style.BarSize.Height.calc(height)};
            f32 const  y {baseline - (yOffset + height) + ((height - barHeight) / 2)};
            f32 const  x {xCursor + ((columnWidth - barWidth) / 2)};

            canvas.set_fill_style(_style.Colors[i % _style.Colors.size()]);

            canvas.begin_path();
            rect_f const block {{x, y}, {barWidth, barHeight}};
            canvas.rounded_rect(block, barRadius);
            canvas.fill();
            canvas.set_stroke_width(1);
            canvas.set_stroke_style(colors::Black);
            canvas.stroke();

            yOffset += height;
        }

        xCursor += columnWidth;
    }
}

////////////////////////////////////////////////////////////

pie_chart::pie_chart(init const& wi)
    : chart {wi}
{
    Class("pie_chart");
}

void pie_chart::on_draw(widget_painter& painter)
{
    rect_f const        rect {draw_background(_style, painter)};
    scissor_guard const guard {painter, this};

    auto& canvas {painter.canvas()};

    auto const seriesCount {Series->size()};
    if (seriesCount == 0) { return; }

    auto const [cx, cy] {rect.center()};
    f32 const maxRadius {std::min(rect.width(), rect.height()) / 2};
    f32 const radiusStep {maxRadius / (seriesCount)};

    for (usize i {seriesCount}; i-- > 0;) {
        usize colorIndex {0};

        auto const& s {Series[i]};
        if (s.Values.empty()) { continue; }

        // total for this series
        f64 total {0.0};
        for (auto const& v : s.Values) { total += v; }
        if (total == 0.0) { continue; }

        f32 const radius {(i + 1) * radiusStep};
        f64       angle {0.0};

        for (auto const& v : s.Values) {
            f64 const fraction {v / total};
            f64 const sweep {fraction * TAU};

            canvas.begin_path();
            canvas.move_to({cx, cy});
            canvas.arc({cx, cy}, radius,
                       radian_d {angle}, radian_d {angle + sweep},
                       gfx::winding::CW);
            canvas.close_path();

            canvas.set_fill_style(_style.Colors[colorIndex % _style.Colors.size()]);
            canvas.fill();
            canvas.set_stroke_width(1);
            canvas.set_stroke_style(colors::Black);
            canvas.stroke();

            angle += sweep;
            ++colorIndex;
        }
    }
}

////////////////////////////////////////////////////////////

void scatter_chart::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    chart_style::Transition(target, from, to, step);

    target.PointSize   = helper::lerp(from.PointSize, to.PointSize, step);
    target.StrokeColor = color::Lerp(from.StrokeColor, to.StrokeColor, step);
}

scatter_chart::scatter_chart(init const& wi)
    : grid_chart {wi}
{
    Class("scatter_chart");
}

void scatter_chart::on_draw(widget_painter& painter)
{
    rect_f const        rect {draw_background(_style, painter)};
    scissor_guard const guard {painter, this};

    auto& canvas {painter.canvas()};

    usize const seriesCount {Series->size()};
    if (seriesCount == 0) { return; }

    draw_grid(canvas, _style, rect);

    // plot points
    for (usize i {0}; i < Series->size(); ++i) {
        auto const& s {Series[i]};
        canvas.set_fill_style(_style.Colors[i % _style.Colors.size()]);
        canvas.set_stroke_style(_style.StrokeColor);
        canvas.set_stroke_width(1);

        canvas.begin_path();
        for (auto const& pt : s.Values) {
            f32 const x {position_in_xaxis(pt.X, XAxis, rect)};
            f32 const y {position_in_yaxis(pt.Y, YAxis, rect)};
            canvas.circle({x, y}, _style.PointSize);
        }
        canvas.fill();
        canvas.stroke();
    }
}

auto scatter_chart::calc_grid_lines() const -> size_i
{
    i32 verticalGridLines {0};
    switch (_style.VerticalGridLines) {
    case grid_line_amount::None:   break;
    case grid_line_amount::Few:    verticalGridLines = ((XAxis->Max - XAxis->Min) / 2) + 1; break;
    case grid_line_amount::Normal: verticalGridLines = ((XAxis->Max - XAxis->Min)) + 1; break;
    case grid_line_amount::Many:   verticalGridLines = ((XAxis->Max - XAxis->Min) * 2) + 1; break;
    }
    i32 horizontalGridLines {0};
    switch (_style.HorizontalGridLines) {
    case grid_line_amount::None:   break;
    case grid_line_amount::Few:    horizontalGridLines = ((YAxis->Max - YAxis->Min) / 2) + 1; break;
    case grid_line_amount::Normal: horizontalGridLines = ((YAxis->Max - YAxis->Min)) + 1; break;
    case grid_line_amount::Many:   horizontalGridLines = ((YAxis->Max - YAxis->Min) * 2) + 1; break;
    }
    return {horizontalGridLines, verticalGridLines};
}

} // namespace display
