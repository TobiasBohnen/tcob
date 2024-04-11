// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/CanvasWidget.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

canvas_widget::canvas_widget(init const& wi)
    : widget {wi}
{
}

void canvas_widget::set_global_composite_operation(composite_operation op)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_global_composite_operation(op); });
}

void canvas_widget::set_global_composite_blendfunc(blend_func sfactor, blend_func dfactor)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_global_composite_blendfunc(sfactor, dfactor); });
}

void canvas_widget::set_global_composite_blendfunc_separate(blend_func srcRGB, blend_func dstRGB, blend_func srcAlpha, blend_func dstAlpha)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_global_composite_blendfunc_separate(srcRGB, dstRGB, srcAlpha, dstAlpha); });
}

void canvas_widget::save()
{
    _commands.emplace_back([](canvas& canvas) { canvas.save(); });
}

void canvas_widget::restore()
{
    _commands.emplace_back([](canvas& canvas) { canvas.restore(); });
}

void canvas_widget::set_fill_style(color c)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_fill_style(c); });
}

void canvas_widget::set_fill_style(canvas_paint const& paint)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_fill_style(paint); });
}

void canvas_widget::set_stroke_style(color c)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_stroke_style(c); });
}

void canvas_widget::set_stroke_style(canvas_paint const& paint)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_stroke_style(paint); });
}

void canvas_widget::set_stroke_width(f32 size)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_stroke_width(size); });
}

void canvas_widget::set_edge_antialias(bool enabled)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_edge_antialias(enabled); });
}

void canvas_widget::set_shape_antialias(bool enabled)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_shape_antialias(enabled); });
}

void canvas_widget::set_miter_limit(f32 limit)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_miter_limit(limit); });
}

void canvas_widget::set_line_cap(line_cap cap)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_line_cap(cap); });
}

void canvas_widget::set_line_join(line_join join)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_line_join(join); });
}

void canvas_widget::set_global_alpha(f32 alpha)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_global_alpha(alpha); });
}

void canvas_widget::begin_path()
{
    _commands.emplace_back([](canvas& canvas) { canvas.begin_path(); });
}

void canvas_widget::close_path()
{
    _commands.emplace_back([](canvas& canvas) { canvas.close_path(); });
}

void canvas_widget::set_path_winding(winding dir)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_path_winding(dir); });
}

void canvas_widget::move_to(point_f pos)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.move_to(pos); });
}

void canvas_widget::line_to(point_f pos)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.line_to(pos); });
}

void canvas_widget::cubic_bezier_to(point_f cp0, point_f cp1, point_f end)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.cubic_bezier_to(cp0, cp1, end); });
}

void canvas_widget::quad_bezier_to(point_f cp, point_f end)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.quad_bezier_to(cp, end); });
}

void canvas_widget::arc_to(point_f pos1, point_f pos2, f32 radius)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.arc_to(pos1, pos2, radius); });
}

void canvas_widget::arc(point_f c, f32 r, radian_f startAngle, radian_f endAngle, gfx::winding dir)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.arc(c, r, startAngle, endAngle, dir); });
}

void canvas_widget::rect(rect_f const& rect)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.rect(rect); });
}

void canvas_widget::rounded_rect(rect_f const& rect, f32 r)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.rounded_rect(rect, r); });
}

void canvas_widget::rounded_rect_varying(rect_f const& rect, f32 radTL, f32 radTR, f32 radBR, f32 radBL)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.rounded_rect_varying(rect, radTL, radTR, radBR, radBL); });
}

void canvas_widget::ellipse(point_f c, f32 rx, f32 ry)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.ellipse(c, rx, ry); });
}

void canvas_widget::circle(point_f c, f32 r)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.circle(c, r); });
}

void canvas_widget::dotted_cubic_bezier(point_f start, point_f cp0, point_f cp1, point_f end, f32 r, i32 numDots)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.dotted_cubic_bezier(start, cp0, cp1, end, r, numDots); });
}

void canvas_widget::dotted_quad_bezier(point_f start, point_f cp, point_f end, f32 r, i32 numDots)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.dotted_quad_bezier(start, cp, end, r, numDots); });
}

void canvas_widget::dotted_line(point_f from, point_f to, f32 r, i32 numDots)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.dotted_line(from, to, r, numDots); });
}

void canvas_widget::dotted_circle(point_f center, f32 rcircle, f32 rdots, i32 numDots)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.dotted_circle(center, rcircle, rdots, numDots); });
}

void canvas_widget::wavy_line(point_f from, point_f to, f32 amp, f32 freq, f32 phase)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.wavy_line(from, to, amp, freq, phase); });
}

void canvas_widget::regular_polygon(point_f pos, size_f size, i32 n)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.regular_polygon(pos, size, n); });
}

void canvas_widget::star(point_f pos, f32 outerR, f32 innerR, i32 n)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.star(pos, outerR, innerR, n); });
}

void canvas_widget::triangle(point_f a, point_f b, point_f c)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.triangle(a, b, c); });
}

void canvas_widget::path_2d(canvas::path2d const& path)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.path_2d(path); });
}

void canvas_widget::translate(point_f c)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.translate(c); });
}

void canvas_widget::rotate(degree_f angle)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.rotate(angle); });
}

void canvas_widget::rotate_at(degree_f angle, point_f p)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.rotate_at(angle, p); });
}

void canvas_widget::scale(size_f scale)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.scale(scale); });
}

void canvas_widget::scale_at(size_f scale, point_f p)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.scale_at(scale, p); });
}

void canvas_widget::skew(degree_f angleX, degree_f angleY)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.skew(angleX, angleY); });
}

void canvas_widget::skew_at(degree_f angleX, degree_f angleY, point_f p)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.skew_at(angleX, angleY, p); });
}

void canvas_widget::set_transform(transform xform)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_transform(xform); });
}

void canvas_widget::reset_transform()
{
    _commands.emplace_back([](canvas& canvas) { canvas.reset_transform(); });
}

void canvas_widget::set_font(font* font)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_font(font); });
}

void canvas_widget::set_text_halign(horizontal_alignment align)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_text_halign(align); });
}

void canvas_widget::set_text_valign(vertical_alignment align)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_text_valign(align); });
}

void canvas_widget::draw_textbox(rect_f const& rect, utf8_string_view text)
{
    _commands.emplace_back([=, str = utf8_string {text}](canvas& canvas) { canvas.draw_textbox(rect, str); });
}

void canvas_widget::fill()
{
    _commands.emplace_back([](canvas& canvas) { canvas.fill(); });
}

void canvas_widget::stroke()
{
    _commands.emplace_back([](canvas& canvas) { canvas.stroke(); });
}

void canvas_widget::clear()
{
    if (!_commands.empty()) {
        _commands.clear();
        force_redraw(get_name() + ": commands cleared.");
    }
}

void canvas_widget::on_paint(widget_painter& painter)
{
    auto& canvas {painter.get_canvas()};
    auto  guard {canvas.create_guard()};

    canvas.set_scissor(Bounds);
    canvas.translate(Bounds->get_position());
    for (auto const& command : _commands) { command(canvas); }
}

void canvas_widget::on_update(milliseconds /* deltaTime */)
{
}

} // namespace display
