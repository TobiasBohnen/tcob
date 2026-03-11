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
    // TODO: Title
    f32 Min {0.0f};
    f32 Max {1.0f};

    f32 SmallStep {0.0f};
    f32 LargeStep {0.5f};

    i32 LabelPrecision {2};

    std::vector<string> CustomLabels;
};

template <typename T>
struct dataset {
    utf8_string Name;
    T           Value;
    color       Color {colors::White};
};

struct legend_def {
    utf8_string Name;
    color       Color;
};

////////////////////////////////////////////////////////////

enum class marker_type : u8 {
    None,
    Disc,
    Square,
    Triangle
};

class TCOB_API marker_element {
public:
    marker_type Type {marker_type::None};
    length      Size {5.0f, length::type::Absolute};
    color       Color {colors::White};
    bool        Filled {true};

    void lerp(marker_element const& from, marker_element const& to, f64 step);

    auto operator==(marker_element const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API chart_style : public widget_style {
public:
    text_element XAxisText;
    length       XLabelHeight {0.05f, length::type::Relative};

    text_element YAxisText;
    length       YLabelWidth {0.05f, length::type::Relative};

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
    auto position_in_xaxis(f32 value, rect_f const& bounds) const -> f32;
    auto position_in_yaxis(f32 value, rect_f const& bounds) const -> f32;

private:
    usize _labelX {0};
    usize _labelY {0};
};

template <typename T>
class chart : public chart_base {
public:
    prop<std::vector<dataset<T>>> Datasets;

    auto legend() const -> std::vector<legend_def> override;

protected:
    explicit chart(init const& wi);

    void on_draw(widget_painter& painter) final;

    void virtual on_draw_chart(widget_painter& painter, std::vector<string> const& xLabels, std::vector<string> const& yLabels) = 0;

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

    length GridLineSize {1.0f, length::type::Absolute};
    color  GridColor {colors::Gray};

    length TickSize {4.0f, length::type::Absolute};

    static void Transition(grid_chart_style& target, grid_chart_style const& from, grid_chart_style const& to, f64 step);
};

template <typename T>
class grid_chart : public chart<T> {
public:
protected:
    grid_chart(widget::init const& wi);

    void draw_grid(gfx::canvas& canvas, grid_chart_style const& style, f32 tickSize, rect_f const& bounds);

    auto grid_rect() const -> rect_f const&;

    virtual auto calc_grid_lines() const -> std::pair<i32, i32> = 0;

private:
    rect_f _gridRect;
};

}

#include "Charting.inl"
