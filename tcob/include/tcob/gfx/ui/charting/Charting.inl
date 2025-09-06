// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Charting.hpp"

#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui::charts {

template <typename T>
inline chart<T>::chart(init const& wi)
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

template <typename T>
inline void chart<T>::on_draw(widget_painter& painter)
{
    if (Series->empty()) { return; }
    auto const* style {dynamic_cast<chart_style const*>(current_style())};
    if (style && style->Colors.empty()) { return; }

    on_draw_chart(painter);
}

template <typename T>
inline auto chart<T>::max_x() const -> usize
{
    return _maxX;
}

template <typename T>
inline auto grid_chart<T>::position_in_xaxis(f32 value, axis const& axis, rect_f const& bounds) const -> f32
{
    f32 const range {axis.Max - axis.Min};
    if (range == 0.0f) { return bounds.left(); }
    f32 const norm {(value - axis.Min) / range};
    return bounds.left() + (norm * bounds.width());
}

template <typename T>
inline auto grid_chart<T>::position_in_yaxis(f32 value, axis const& axis, rect_f const& bounds) const -> f32
{
    f32 const range {axis.Max - axis.Min};
    if (range == 0.0f) { return bounds.bottom(); }
    f32 const norm {(value - axis.Min) / range};
    return bounds.bottom() - (norm * bounds.height());
}

////////////////////////////////////////////////////////////

template <typename T>
inline grid_chart<T>::grid_chart(widget::init const& wi)
    : chart<T> {wi}
{
}

template <typename T>
inline void grid_chart<T>::draw_grid(gfx::canvas& canvas, grid_chart_style const& style, rect_f const& bounds) const
{
    auto [horizontalGridLines, verticalGridLines] {calc_grid_lines()};
    if (verticalGridLines == 1) { verticalGridLines = 2; }
    if (verticalGridLines == 1) { verticalGridLines = 2; }

    canvas.set_stroke_style(style.GridColor);
    canvas.set_stroke_width(style.GridLineWidth);

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
}

}
