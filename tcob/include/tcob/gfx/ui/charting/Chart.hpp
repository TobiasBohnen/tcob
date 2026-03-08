// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <utility>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/input/Input.hpp"
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
        bool   SmoothLines {false};

        marker_type MarkerType {marker_type::None};
        length      MarkerSize {5.0f, length::type::Absolute};
        color       MarkerColor {colors::White};
        bool        MarkerFilled {true};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit line_chart(init const& wi);

protected:
    void on_draw_chart(widget_painter& painter, std::vector<string> const& xLabels, std::vector<string> const& yLabels) override;

    auto calc_grid_lines() const -> std::pair<i32, i32> override;

private:
    line_chart::style _style;
};

////////////////////////////////////////////////////////////

class TCOB_API bar_chart : public grid_chart<std::vector<f32>> {
public:
    class TCOB_API style : public grid_chart_style {
    public:
        length BarSize {1.f, length::type::Relative};
        length BarRadius {0, length::type::Relative};
        bool   StackBars {false};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit bar_chart(init const& wi);

protected:
    void on_draw_chart(widget_painter& painter, std::vector<string> const& xLabels, std::vector<string> const& yLabels) override;

    auto calc_grid_lines() const -> std::pair<i32, i32> override;

private:
    bar_chart::style _style;
};

////////////////////////////////////////////////////////////

class TCOB_API marimekko_chart : public chart<std::vector<f32>> {
public:
    class TCOB_API style : public chart_style {
    public:
        dimensions BarSize {};
        length     BarRadius {0, length::type::Relative};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit marimekko_chart(init const& wi);

protected:
    void on_draw_chart(widget_painter& painter, std::vector<string> const& xLabels, std::vector<string> const& yLabels) override;

private:
    marimekko_chart::style _style;
};

////////////////////////////////////////////////////////////

class TCOB_API pie_chart : public chart<f32> {
public:
    class TCOB_API style : public chart_style {
    public:
        length   InnerRadius {0, length::type::Relative};
        degree_f PadAngle {0};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit pie_chart(init const& wi);

protected:
    void on_draw_chart(widget_painter& painter, std::vector<string> const& xLabels, std::vector<string> const& yLabels) override;

private:
    pie_chart::style _style;
};

////////////////////////////////////////////////////////////

class TCOB_API scatter_chart : public grid_chart<std::vector<point_f>> {
public:
    class TCOB_API style : public grid_chart_style {
    public:
        length PointSize {3.0f, length::type::Absolute};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit scatter_chart(init const& wi);

protected:
    void on_draw_chart(widget_painter& painter, std::vector<string> const& xLabels, std::vector<string> const& yLabels) override;

    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;
    void on_double_click() override;

    auto calc_grid_lines() const -> std::pair<i32, i32> override;

private:
    struct axis_view {
        f32 Min {}, Max {}, SmallStep {}, LargeStep {};
    };

    scatter_chart::style _style;
    point_f              _dragAccum {};
    bool                 _ignoreAxisChange {false};
    axis_view            _initialX {}, _initialY {};
};

////////////////////////////////////////////////////////////

class TCOB_API radar_chart : public chart<std::vector<f32>> {
public:
    class TCOB_API style : public chart_style {
    public:
        u8 FillAreaAlpha {0};

        length LineSize {3.0f, length::type::Absolute};

        grid_line_amount GridLines {grid_line_amount::Normal};
        length           GridLineSize {2.0f, length::type::Absolute};
        color            GridColor {colors::Gray};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit radar_chart(init const& wi);

protected:
    void on_draw_chart(widget_painter& painter, std::vector<string> const& xLabels, std::vector<string> const& yLabels) override;

private:
    radar_chart::style _style;
};

}
