// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Charting.hpp"

#include <algorithm>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui::charts {

void chart::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.HorizontalGridLines = helper::lerp(from.HorizontalGridLines, to.HorizontalGridLines, step);
    target.VerticalGridLines   = helper::lerp(from.VerticalGridLines, to.VerticalGridLines, step);

    target.GridLineWidth = helper::lerp(from.GridLineWidth, to.GridLineWidth, step);
    target.GridColor     = color::Lerp(from.GridColor, to.GridColor, step);

    target.Colors.clear();
    target.Colors.resize(from.Colors.size());
    for (usize i {0}; i < from.Colors.size(); ++i) {
        if (i >= to.Colors.size()) {
            target.Colors[i] = from.Colors[i];
        } else {
            target.Colors[i] = color::Lerp(from.Colors[i], to.Colors[i], step);
        }
    }
}

chart::chart(init const& wi)
    : widget {wi}
{
    Series.Changed.connect([&] {
        _maxX = 0;
        for (auto const& s : *Series) {
            _maxX = std::max(_maxX, s.Values.size());
        }
        queue_redraw();
    });
}

void chart::draw_grid(gfx::canvas& canvas, style const& style, rect_f const& bounds) const
{
    canvas.set_stroke_style(style.GridColor);
    canvas.set_stroke_width(style.GridLineWidth);

    // horizontal lines
    if (style.HorizontalGridLines > 1) {
        for (i32 i {0}; i < style.HorizontalGridLines; ++i) {
            f32 const t {static_cast<f32>(i) / static_cast<f32>(style.HorizontalGridLines - 1)};
            f32 const y {bounds.bottom() - (t * bounds.height())};
            canvas.stroke_line({bounds.left(), y}, {bounds.right(), y});
        }
    }

    // vertical lines
    if (style.VerticalGridLines > 1) {
        for (i32 i {0}; i < style.VerticalGridLines; ++i) {
            f32 const t {static_cast<f32>(i) / static_cast<f32>(style.VerticalGridLines - 1)};
            f32 const x {bounds.right() - (t * bounds.width())};
            canvas.stroke_line({x, bounds.top()}, {x, bounds.bottom()});
        }
    }
}

auto chart::max_x() const -> usize
{
    return _maxX;
}

auto point_y(f32 value, axis const& axis, rect_f const& bounds) -> f32
{
    f32 const range {axis.Max - axis.Min};
    if (range == 0.0f) { return bounds.bottom(); }
    f32 const norm {(value - axis.Min) / range};
    return bounds.bottom() - (norm * bounds.height());
}

////////////////////////////////////////////////////////////

void line_chart::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    chart::style::Transition(target, from, to, step);

    target.LineSize = length::Lerp(from.LineSize, to.LineSize, step);
}

line_chart::line_chart(init const& wi)
    : chart {wi}
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
            f32 const y {point_y(s.Values[j], *YAxis, rect)};
            points.emplace_back(x, y);
        }

        if (points.size() == 1) { continue; }

        canvas.begin_path();

        canvas.move_to(points[0]);
        for (u32 i {1}; i < points.size(); ++i) {
            canvas.line_to(points[i]);
        }

        canvas.set_stroke_style(colors::Black);
        canvas.set_stroke_width(_style.LineSize.calc(rect.width()) + 1);
        canvas.stroke();
        canvas.set_stroke_style(_style.Colors[i % _style.Colors.size()]);
        canvas.set_stroke_width(_style.LineSize.calc(rect.width()));
        canvas.stroke();
    }
}

////////////////////////////////////////////////////////////

void bar_chart::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    chart::style::Transition(target, from, to, step);

    target.BarSize = length::Lerp(from.BarSize, to.BarSize, step);
}

bar_chart::bar_chart(init const& wi)
    : chart {wi}
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

    auto const xStep {rect.width() / max_x()};
    auto const xWidth {_style.BarSize.calc(xStep)};

    for (usize i {0}; i < barCount; ++i) {
        auto const& s {Series[i]};
        usize const valueCount {s.Values.size()};
        if (valueCount == 0) { return; }

        canvas.set_fill_style(_style.Colors[i % _style.Colors.size()]);

        f32 const xOffset {rect.left() + (xWidth / barCount * i) + ((xStep - xWidth) / 2)};
        for (usize j {0}; j < valueCount; ++j) {
            f32 const x {xOffset + (xStep * j)};
            f32 const y {point_y(s.Values[j], *YAxis, rect)};

            canvas.begin_path();
            canvas.rect({{x, y}, {xWidth / barCount, rect.bottom() - y}});
            canvas.fill();
            canvas.set_stroke_width(1);
            canvas.set_stroke_style(colors::Black);
            canvas.stroke();
        }
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

} // namespace display
