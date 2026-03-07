// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <utility>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui::charts {
////////////////////////////////////////////////////////////

struct axis {
    // TODO: Title, Ticks
    f32 Min {0.0f};
    f32 Max {1.0f};

    i32 LabelCount {0};
    i32 LabelPrecision {2};

    std::vector<string> CustomLabels;
};

template <typename T>
struct datapoint {
    utf8_string Name;
    T           Value;
};

struct legend_def {
    utf8_string Name;
    color       Color;
};

////////////////////////////////////////////////////////////

class TCOB_API chart_style : public widget_style {
public:
    text_element XAxisText;
    length       XLabelHeight;

    text_element YAxisText;
    length       YLabelWidth;

    std::vector<color> Colors;

    color  OutlineColor {colors::Black};
    length OutlineSize {1.0f, length::type::Absolute};

    static void Transition(chart_style& target, chart_style const& from, chart_style const& to, f64 step);
};

class TCOB_API chart_base : public widget {
public:
    virtual ~chart_base() = default;

    prop<axis> XAxis;
    prop<axis> YAxis;

    virtual auto legend() const -> std::vector<legend_def> = 0;

protected:
    explicit chart_base(init const& wi);

    auto x_label_count() const -> usize;
    auto y_label_count() const -> usize;

private:
    usize _labelX {0};
    usize _labelY {0};
};

template <typename T>
class chart : public chart_base {
public:
    prop<std::vector<datapoint<T>>> Dataset;

    auto legend() const -> std::vector<legend_def> override;

protected:
    explicit chart(init const& wi);

    void on_draw(widget_painter& painter) final;

    void virtual on_draw_chart(widget_painter& painter, std::vector<string> const& xLabels, std::vector<string> const& yLabels) = 0;
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

    length GridLineSize {1.0f, length::type::Absolute};
    color  GridColor {colors::Gray};

    static void Transition(grid_chart_style& target, grid_chart_style const& from, grid_chart_style const& to, f64 step);
};

template <typename T>
class grid_chart : public chart<T> {
public:
protected:
    grid_chart(widget::init const& wi);

    void draw_grid(gfx::canvas& canvas, grid_chart_style const& style, rect_f const& bounds);

    auto grid_rect() const -> rect_f const&;

    virtual auto calc_grid_lines() const -> std::pair<i32, i32> = 0;

    auto position_in_xaxis(f32 value, axis const& axis, rect_f const& bounds) const -> f32;
    auto position_in_yaxis(f32 value, axis const& axis, rect_f const& bounds) const -> f32;

private:
    rect_f _gridRect;
};

}

#include "Charting.inl"
