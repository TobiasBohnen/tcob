// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui::charts {
////////////////////////////////////////////////////////////

struct axis {
    f32 Min {0.0f};
    f32 Max {1.0f};
};

template <typename T>
struct datapoint {
    utf8_string Name;
    T           Value;
};

////////////////////////////////////////////////////////////

class TCOB_API chart_style : public widget_style {
public:
    std::vector<color> Colors;

    static void Transition(chart_style& target, chart_style const& from, chart_style const& to, f64 step);
};

template <typename T>
class chart : public widget {
public:
    prop<std::vector<datapoint<T>>> Dataset;

protected:
    explicit chart(init const& wi);

    void on_draw(widget_painter& painter) final;

    void virtual on_draw_chart(widget_painter& painter) = 0;

    auto max_x() const -> usize;

private:
    usize _maxX {0};
};

////////////////////////////////////////////////////////////

enum class grid_line_amount : u8 {
    None,
    Few,
    Normal,
    Many
};

class TCOB_API grid_chart_style : public chart_style {
public:
    grid_line_amount HorizontalGridLines {grid_line_amount::Normal};
    grid_line_amount VerticalGridLines {grid_line_amount::Normal};

    f32   GridLineWidth {1.0f};
    color GridColor {colors::Gray};

    static void Transition(grid_chart_style& target, grid_chart_style const& from, grid_chart_style const& to, f64 step);
};

template <typename T>
class grid_chart : public chart<T> {
public:
protected:
    grid_chart(widget::init const& wi);

    void draw_grid(gfx::canvas& canvas, grid_chart_style const& style, rect_f const& bounds) const;

    virtual auto calc_grid_lines() const -> size_i = 0;

    auto position_in_xaxis(f32 value, axis const& axis, rect_f const& bounds) const -> f32;
    auto position_in_yaxis(f32 value, axis const& axis, rect_f const& bounds) const -> f32;
};

}

#include "Charting.inl"
