// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/charting/Charting.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui::charts {
////////////////////////////////////////////////////////////

class TCOB_API line_chart : public grid_chart<std::vector<f32>> {
public:
    class TCOB_API style : public grid_chart_style {
    public:
        length LineSize {3.0f, length::type::Absolute};
        bool   SmoothLines {false}; // TODO: lerp?

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit line_chart(init const& wi);

    prop<axis> YAxis;

protected:
    void on_draw_chart(widget_painter& painter) override;

    auto calc_grid_lines() const -> size_i override;

private:
    style _style;
};

////////////////////////////////////////////////////////////

class TCOB_API bar_chart : public grid_chart<std::vector<f32>> {
public:
    class TCOB_API style : public grid_chart_style {
    public:
        length BarSize {1.f, length::type::Relative};
        length BarRadius {};
        bool   StackBars {false}; // TODO: lerp?

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit bar_chart(init const& wi);

    prop<axis> YAxis;

protected:
    void on_draw_chart(widget_painter& painter) override;

    auto calc_grid_lines() const -> size_i override;

private:
    style _style;
};

////////////////////////////////////////////////////////////

class TCOB_API marimekko_chart : public chart<std::vector<f32>> {
public:
    class TCOB_API style : public grid_chart_style {
    public:
        dimensions BarSize {};
        length     BarRadius {};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit marimekko_chart(init const& wi);

protected:
    void on_draw_chart(widget_painter& painter) override;

private:
    style _style;
};

////////////////////////////////////////////////////////////

class TCOB_API pie_chart : public chart<f32> {
public:
    class TCOB_API style : public chart_style {
    public:
    };

    explicit pie_chart(init const& wi);

protected:
    void on_draw_chart(widget_painter& painter) override;

private:
    style _style;
};

////////////////////////////////////////////////////////////

class TCOB_API scatter_chart : public grid_chart<std::vector<point_f>> {
public:
    class TCOB_API style : public grid_chart_style {
    public:
        f32   PointSize {4.0f};
        color StrokeColor {colors::Black};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit scatter_chart(init const& wi);

    prop<axis> XAxis;
    prop<axis> YAxis;

protected:
    void on_draw_chart(widget_painter& painter) override;

    auto calc_grid_lines() const -> size_i override;

private:
    style _style;
};

////////////////////////////////////////////////////////////

class TCOB_API radar_chart : public chart<std::vector<f32>> {
public:
    class style : public chart_style {
    public:
        f32 LineWidth {4.0f};
        u8  FillAreaAlpha {0};

        grid_line_amount GridLines {grid_line_amount::Normal};
        f32              GridLineWidth {2.0f};
        color            GridColor {colors::Gray};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit radar_chart(init const& wi);

    prop<axis> ValueAxis;

protected:
    void on_draw_chart(widget_painter& painter) override;

private:
    style _style;
};

}
