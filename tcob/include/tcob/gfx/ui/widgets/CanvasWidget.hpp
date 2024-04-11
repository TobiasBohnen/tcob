// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Size.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API canvas_widget : public widget {
public:
    explicit canvas_widget(init const& wi);

    void clear();

    void set_global_composite_operation(composite_operation op);
    void set_global_composite_blendfunc(blend_func sfactor, blend_func dfactor);
    void set_global_composite_blendfunc_separate(blend_func srcRGB, blend_func dstRGB, blend_func srcAlpha, blend_func dstAlpha);

    // State handling
    void save();
    void restore();

    // Render styles
    void set_fill_style(color c);
    void set_fill_style(canvas_paint const& paint);
    void set_stroke_style(color c);
    void set_stroke_style(canvas_paint const& paint);
    void set_stroke_width(f32 size);
    void set_edge_antialias(bool enabled);
    void set_shape_antialias(bool enabled);
    void set_miter_limit(f32 limit);
    void set_line_cap(line_cap cap);
    void set_line_join(line_join join);
    void set_global_alpha(f32 alpha);

    // Paths
    void begin_path();
    void close_path();
    void set_path_winding(winding dir);
    void move_to(point_f pos);
    void line_to(point_f pos);
    void cubic_bezier_to(point_f cp0, point_f cp1, point_f end);
    void quad_bezier_to(point_f cp, point_f end);
    void arc_to(point_f pos1, point_f pos2, f32 radius);
    void arc(point_f c, f32 r, radian_f startAngle, radian_f endAngle, winding dir);
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

    void path_2d(canvas::path2d const& path);

    void fill();
    void stroke();

    // Transforms
    void translate(point_f c);
    void rotate(degree_f angle);
    void rotate_at(degree_f angle, point_f p);
    void scale(size_f scale);
    void scale_at(size_f scale, point_f p);
    void skew(degree_f angleX, degree_f angleY);
    void skew_at(degree_f angleX, degree_f angleY, point_f p);
    void set_transform(transform xform);
    void reset_transform();

    // Font
    void set_font(font* font);
    void set_text_halign(horizontal_alignment align);
    void set_text_valign(vertical_alignment align);

    void draw_textbox(rect_f const& rect, utf8_string_view text);

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

private:
    std::vector<std::function<void(canvas&)>> _commands;
};

}
