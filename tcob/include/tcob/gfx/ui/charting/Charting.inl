// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Charting.hpp"

#include <cmath>
#include <format>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui::charts {

template <typename T>
inline chart<T>::chart(init const& wi)
    : chart_base {wi}
{
    Datasets.Changed.connect([&] {
        _maxX = 0;
        if constexpr (HasSize<T>) {
            for (auto const& s : *Datasets) {
                _maxX = std::max(_maxX, s.Value.size());
            }
        } else {
            _maxX = 1;
        }
        queue_redraw();
    });
}

template <typename T>
inline auto chart<T>::legend() const -> std::vector<legend_def>
{
    std::vector<legend_def> retValue;

    auto const* style {dynamic_cast<chart_style const*>(current_style())};
    for (usize i {0}; i < Datasets->size() && style; ++i) {
        auto const& s {Datasets[i]};
        retValue.emplace_back(s.Name, s.Color, s.Marker);
    }
    return retValue;
}

template <typename T>
inline void chart<T>::on_draw(widget_painter& painter)
{
    if (Datasets->empty()) { return; }
    auto const* style {dynamic_cast<chart_style const*>(current_style())};
    if (!style) { return; }

    auto const getLabels {[&](axis const& a) -> std::vector<string> {
        if (!a.CustomLabels.empty()) { return a.CustomLabels; }
        if (a.LargeStep <= 0.0f) { return {}; }
        std::vector<string> labels;
        i32 const           count {static_cast<i32>(std::round((a.Max - a.Min) / a.LargeStep)) + 1};
        for (i32 i {0}; i < count; ++i) {
            f32 const value {a.Min + (static_cast<f32>(i) * a.LargeStep)};
            labels.push_back(std::format("{:.{}f}", value, a.LabelPrecision));
        }
        return labels;
    }};

    on_draw_chart(painter, getLabels(XAxis), getLabels(YAxis));
}

template <typename T>
inline void chart<T>::draw_marker(gfx::canvas& canvas, point_f p, dataset<T> const& s, f32 markerSize, f32 outlineWidth)
{
    marker const& marker {s.Marker};
    color const   markerColor {s.Marker.Color.value_or(s.Color)};
    color const   outlineColor {s.Marker.OutlineColor.value_or(s.OutlineColor)};

    draw_chart_marker(canvas, p, marker, markerColor, markerSize, outlineColor, outlineWidth);
}

template <typename T>
inline auto chart<T>::max_x() const -> usize
{
    return _maxX;
}

////////////////////////////////////////////////////////////

template <typename T>
inline grid_chart<T>::grid_chart(widget::init const& wi)
    : chart<T> {wi}
{
}

template <typename T>
inline void grid_chart<T>::draw_grid(gfx::canvas& canvas, grid_chart_style const& style, f32 tickSize, rect_f const& bounds)
{
    _gridRect = bounds;
    auto [horizontalGridLines, verticalGridLines] {calc_grid_lines()};

    canvas.set_stroke_style(style.GridColor);
    canvas.set_stroke_width(style.GridLineSize.calc(bounds.width()));

    // horizontal lines
    if (horizontalGridLines > 1) {
        for (i32 i {0}; i < horizontalGridLines; ++i) {
            f32 const t {static_cast<f32>(i) / static_cast<f32>(horizontalGridLines - 1)};
            f32 const y {bounds.bottom() - (t * bounds.height())};
            canvas.stroke_line({bounds.left(), y}, {bounds.right(), y});
        }
    }

    // vertical lines
    if (verticalGridLines > 1) {
        for (i32 i {0}; i < verticalGridLines; ++i) {
            f32 const t {static_cast<f32>(i) / static_cast<f32>(verticalGridLines - 1)};
            f32 const x {bounds.right() - (t * bounds.width())};
            canvas.stroke_line({x, bounds.top()}, {x, bounds.bottom()});
        }
    }

    // ticks
    if (this->XAxis->SmallStep > 0.0f) {
        f32 const xRange {this->XAxis->Max - this->XAxis->Min};
        i32 const xTickCount {static_cast<i32>(std::round(xRange / this->XAxis->SmallStep)) + 1};
        for (i32 i {0}; i < xTickCount; ++i) {
            f32 const t {static_cast<f32>(i) / static_cast<f32>(xTickCount - 1)};
            f32 const x {bounds.left() + (t * bounds.width())};
            canvas.stroke_line({x, bounds.bottom()}, {x, bounds.bottom() + tickSize});
        }
    }

    if (this->YAxis->SmallStep > 0.0f) {
        f32 const yRange {this->YAxis->Max - this->YAxis->Min};
        i32 const yTickCount {static_cast<i32>(std::round(yRange / this->YAxis->SmallStep)) + 1};
        for (i32 i {0}; i < yTickCount; ++i) {
            f32 const t {static_cast<f32>(i) / static_cast<f32>(yTickCount - 1)};
            f32 const y {bounds.bottom() - (t * bounds.height())};
            canvas.stroke_line({bounds.left() - tickSize, y}, {bounds.left(), y});
        }
    }
}

template <typename T>
inline auto grid_chart<T>::grid_rect() const -> rect_f const&
{
    return _gridRect;
}

}
