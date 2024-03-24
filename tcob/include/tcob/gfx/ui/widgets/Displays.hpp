// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <bitset>
#include <vector>

#include "tcob/core/FlatMap.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {

////////////////////////////////////////////////////////////

class TCOB_API dot_matrix_display : public widget {
public:
    struct dot {
        enum class type {
            Disc,
            Square
        };

        length               Size;
        flat_map<u16, color> Colors;
        type                 Type {type::Disc};
    };

    class TCOB_API style : public background_style {
    public:
        dot Dot;
    };

    explicit dot_matrix_display(init const& wi);

    prop<size_i>           Size;
    prop<std::vector<u16>> Dots;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;
};

////////////////////////////////////////////////////////////

class TCOB_API seven_segment_display : public widget {
public:
    struct segment {
        length Size;
        color  ActiveColor {colors::Black};
        color  InactiveColor {colors::Transparent};
    };

    struct style : public background_style {
        segment Segment;
    };

    explicit seven_segment_display(init const& wi);

    prop<string> Text;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

private:
    auto get_segment(char c) -> std::bitset<7>;
};

////////////////////////////////////////////////////////////

class TCOB_API canvas_widget : public widget {
public:
    explicit canvas_widget(init const& wi);

    void set_fill_style(color c);
    void set_fill_style(canvas_paint const& paint);
    void set_stroke_style(color c);
    void set_stroke_style(canvas_paint const& paint);
    void set_stroke_width(f32 size);
    void set_line_cap(line_cap cap);
    void set_line_join(line_join join);

    void begin_path();
    void close_path();

    void move_to(point_f pos);
    void line_to(point_f pos);
    void cubic_bezier_to(point_f cp0, point_f cp1, point_f end);
    void quad_bezier_to(point_f cp, point_f end);
    void arc_to(point_f pos1, point_f pos2, f32 radius);
    void arc(point_f c, f32 r, radian_f startAngle, radian_f endAngle, gfx::winding dir);
    void rect(rect_f const& rect);
    void rounded_rect(rect_f const& rect, f32 r);
    void rounded_rect_varying(rect_f const& rect, f32 radTL, f32 radTR, f32 radBR, f32 radBL);
    void ellipse(point_f c, f32 rx, f32 ry);
    void circle(point_f c, f32 r);

    void dotted_cubic_bezier(point_f start, point_f cp0, point_f cp1, point_f end, f32 r, i32 numDots);
    void dotted_quad_bezier(point_f start, point_f cp, point_f end, f32 r, i32 numDots);
    void dotted_line(point_f from, point_f to, f32 r, i32 numDots);
    void dotted_circle(point_f center, f32 rcircle, f32 rdots, i32 numDots);
    void wavy_line(point_f from, point_f to, f32 amp, f32 freq, f32 phase);
    void regular_polygon(point_f pos, size_f size, i32 n);
    void star(point_f pos, f32 outerR, f32 innerR, i32 n);
    void triangle(point_f a, point_f b, point_f c);

    void fill();
    void stroke();

    void clear();

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

private:
    std::vector<std::function<void(canvas&)>> _commands;
};

}
