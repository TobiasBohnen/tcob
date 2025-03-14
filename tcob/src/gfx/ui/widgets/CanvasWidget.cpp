// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/CanvasWidget.hpp"

#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Path2d.hpp"
#include "tcob/gfx/Texture.hpp"
#include "tcob/gfx/Transform.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

using namespace gfx;

canvas_widget::canvas_widget(init const& wi)
    : widget {wi}
{
    Class("canvas_widget");
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

void canvas_widget::set_fill_style(canvas::paint const& paint)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_fill_style(paint); });
}

void canvas_widget::set_stroke_style(color c)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_stroke_style(c); });
}

void canvas_widget::set_stroke_style(canvas::paint const& paint)
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

void canvas_widget::set_line_dash(std::vector<f32> const& dashPatternRel)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_line_dash(dashPatternRel); });
}

void canvas_widget::set_dash_offset(f32 offset)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_dash_offset(offset); });
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

void canvas_widget::wavy_line_to(point_f to, f32 amp, f32 freq, f32 phase)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.wavy_line_to(to, amp, freq, phase); });
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

void canvas_widget::path_2d(path2d const& path)
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

void canvas_widget::set_scissor(rect_f const& rect, bool transform)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_scissor(rect, transform); });
}

void canvas_widget::reset_scissor()
{
    _commands.emplace_back([](canvas& canvas) { canvas.reset_scissor(); });
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

void canvas_widget::draw_text(rect_f const& rect, utf8_string_view text)
{
    _commands.emplace_back([=, str = utf8_string {text}](canvas& canvas) { canvas.draw_text(rect, str); });
}

void canvas_widget::fill()
{
    _commands.emplace_back([](canvas& canvas) { canvas.fill(); });
}

void canvas_widget::stroke()
{
    _commands.emplace_back([](canvas& canvas) { canvas.stroke(); });
}

void canvas_widget::clip()
{
    _commands.emplace_back([](canvas& canvas) { canvas.clip(); });
}

void canvas_widget::draw_image(texture* image, string const& region, rect_f const& rect)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.draw_image(image, region, rect); });
}

void canvas_widget::clear()
{
    if (!_commands.empty()) {
        _commands.clear();
        request_redraw(this->name() + ": commands cleared.");
    }
}

void canvas_widget::on_draw(widget_painter& painter)
{
    auto& canvas {painter.canvas()};
    auto  guard {canvas.create_guard()};

    canvas.set_scissor(Bounds());
    canvas.translate(Bounds->Position);
    for (auto const& command : _commands) { command(canvas); }
}

void canvas_widget::on_update(milliseconds /* deltaTime */)
{
}

} // namespace display
