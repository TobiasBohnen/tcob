// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/core/Property.hpp"
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Rect.hpp"
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

struct series {
    utf8_string      Name;
    std::vector<f32> Values;
};

enum class grid_line_amount : u8 {
    None,
    Few,
    Normal,
    Many
};

////////////////////////////////////////////////////////////

class TCOB_API chart : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        grid_line_amount HorizontalGridLines {grid_line_amount::Normal};
        grid_line_amount VerticalGridLines {grid_line_amount::Normal};

        f32   GridLineWidth {1.0f};
        color GridColor {colors::Gray};

        std::vector<color> Colors;

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    prop<std::vector<series>> Series;

protected:
    explicit chart(init const& wi);

    void draw_grid(gfx::canvas& canvas, style const& style, rect_f const& bounds, i32 verticalGridLines) const;

    auto max_x() const -> usize;

private:
    usize _maxX {0};
};

////////////////////////////////////////////////////////////

class TCOB_API line_chart : public chart {
public:
    class TCOB_API style : public chart::style {
    public:
        length LineSize {3.0f, length::type::Absolute};
        bool   SmoothLines {false};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit line_chart(init const& wi);

    prop<axis> YAxis;

protected:
    void on_draw(widget_painter& painter) override;

private:
    style _style;
};

////////////////////////////////////////////////////////////

class TCOB_API bar_chart : public chart {
public:
    class TCOB_API style : public chart::style {
    public:
        length BarSize {1.f, length::type::Relative};
        length BarRadius {};
        bool   StackBars {false};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit bar_chart(init const& wi);

    prop<axis> YAxis;

protected:
    void on_draw(widget_painter& painter) override;

private:
    style _style;
};

////////////////////////////////////////////////////////////

class TCOB_API pie_chart : public chart {
public:
    class TCOB_API style : public chart::style {
    public:
    };

    explicit pie_chart(init const& wi);

protected:
    void on_draw(widget_painter& painter) override;

private:
    style _style;
};

////////////////////////////////////////////////////////////

}
