// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Displays.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

dot_matrix_display::dot_matrix_display(init const& wi)
    : widget {wi}
{
    Size.Changed.connect([&](auto const&) { force_redraw(get_name() + ": Size changed"); });
    Dots.Changed.connect([&](auto const&) { force_redraw(get_name() + ": Dots changed"); });

    Class("dot_matrix_display");
}

void dot_matrix_display::on_paint(widget_painter& painter)
{
    if (Size->Width <= 0 || Size->Height <= 0) {
        return;
    }

    if (auto const style {get_style<dot_matrix_display::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        auto& canvas {painter.get_canvas()};
        canvas.save();

        f32 const width {style->Dot.Size.calc(rect.Width)};
        for (i32 idx {0}; idx < Size->Width * Size->Height && idx < std::ssize(*Dots); ++idx) {
            rect_f const dotRect {(idx % Size->Width) * width + rect.left(),
                                  (idx / Size->Width) * width + rect.top(),
                                  width, width};

            switch (style->Dot.Type) {
            case dot::type::Disc: {
                canvas.set_fill_style(style->Dot.Colors[Dots->at(idx)]);
                canvas.fill_circle(dotRect.get_center(), width / 2);
            } break;
            case dot::type::Square: {
                canvas.set_fill_style(style->Dot.Colors[Dots->at(idx)]);
                canvas.fill_rect(dotRect);
            } break;
            }
        }

        canvas.restore();
    }
}

void dot_matrix_display::on_update(milliseconds /* deltaTime */)
{
}

////////////////////////////////////////////////////////////

seven_segment_display::seven_segment_display(init const& wi)
    : widget {wi}
{
    Text.Changed.connect([&](auto const&) { force_redraw(get_name() + ": text changed"); });

    Class("seven_segment_display");
}

auto seven_segment_display::get_segment(char c) -> std::bitset<7>
{
    switch (c) {
    case '0':
    case 'D':
    case 'O':
        return {0b1110111};
    case '1':
    case 'I':
        return {0b0010010};
    case '2':
    case 'Z':
        return {0b1011101};
    case '3':
        return {0b1011011};
    case '4':
        return {0b0111010};
    case '5':
    case 'S':
        return {0b1101011};
    case '6':
        return {0b1101111};
    case '7':
        return {0b1010010};
    case '8':
    case 'B':
        return {0b1111111};
    case '9':
        return {0b1111011};
    case 'A':
        return {0b1111110};
    case 'C':
        return {0b1100101};
    case 'E':
        return {0b1101101};
    case 'F':
        return {0b1101100};
    case 'G':
        return {0b1101111};
    case 'H':
        return {0b0111110};
    case 'J':
        return {0b0010111};
    case 'L':
        return {0b0100101};
    case 'P':
        return {0b1111100};
    case 'U':
        return {0b0110111};
    case '-':
        return {0b0001000};
    case '"':
        return {0b0110000};
    case '\'':
        return {0b0100000};
    case ',':
        return {0b0000100};
    case '=':
        return {0b1001000};
    case ' ':
    default:
        return {0b0000000};
    }
    /*
    000 AAA
    1 2 F B
    333 GGG
    4 5 E C
    666 DDD
    */
}

void seven_segment_display::on_paint(widget_painter& painter)
{
    if (auto const style {get_style<seven_segment_display::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        auto& canvas {painter.get_canvas()};
        canvas.save();

        f32 const width {style->Segment.Size.calc(rect.Width)};
        f32 const thick {width / 4};

        point_f offset {rect.top_left()};
        point_f offset2 {1, 0};

        static std::array<point_f, 18> points {{
            {0, 0},                             // 01
            {width, 0},                         // 02
            {width - thick, thick},             // 03
            {thick, thick},                     // 04

            {0, width - thick / 2},             // 05
            {thick, width - thick / 2},         // 06
            {thick / 2, width},                 // 07

            {0, width + thick / 2},             // 08
            {thick, width + thick / 2},         // 09
            {0, width * 2},                     // 10
            {thick, width * 2 - thick},         // 11

            {width - thick, width * 2 - thick}, // 12
            {width, width * 2},                 // 13

            {width - thick, width + thick / 2}, // 14
            {width, width + thick / 2},         // 15
            {width - thick / 2, width},         // 16

            {width - thick, width - thick / 2}, // 17
            {width, width - thick / 2},         // 18
        }};

        for (auto const c : Text()) {
            auto segments {get_segment(c)};

            for (i32 i {0}; i < 7; i++) {
                if (segments[6 - i]) {
                    canvas.set_fill_style(style->Segment.ActiveColor);
                } else {
                    if (style->Segment.InactiveColor.A == 0) {
                        continue;
                    }
                    canvas.set_fill_style(style->Segment.InactiveColor);
                }

                switch (i) {
                case 0:
                    canvas.fill_lines({{points[0] + offset + offset2, points[1] + offset - offset2, points[2] + offset - offset2, points[3] + offset + offset2}});
                    break;
                case 1:
                    canvas.fill_lines({{points[0] + offset, points[3] + offset, points[5] + offset, points[6] + offset, points[4] + offset}});
                    break;
                case 2:
                    canvas.fill_lines({{points[2] + offset, points[1] + offset, points[17] + offset, points[15] + offset, points[16] + offset}});
                    break;
                case 3:
                    canvas.fill_lines({{points[6] + offset + offset2, points[5] + offset + offset2, points[16] + offset - offset2, points[15] + offset - offset2, points[13] + offset - offset2, points[8] + offset + offset2}});
                    break;
                case 4:
                    canvas.fill_lines({{points[6] + offset, points[8] + offset, points[10] + offset, points[9] + offset, points[7] + offset}});
                    break;
                case 5:
                    canvas.fill_lines({{points[15] + offset, points[14] + offset, points[12] + offset, points[11] + offset, points[13] + offset}});
                    break;
                case 6:
                    canvas.fill_lines({{points[10] + offset + offset2, points[11] + offset - offset2, points[12] + offset - offset2, points[9] + offset + offset2}});
                    break;
                }
            }
            offset.X += width + thick;
        }

        canvas.restore();
    }
}

void seven_segment_display::on_update(milliseconds /* deltaTime */)
{
}

////////////////////////////////////////////////////////////

canvas_widget::canvas_widget(init const& wi)
    : widget {wi}
{
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

void canvas_widget::set_line_cap(line_cap cap)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_line_cap(cap); });
}

void canvas_widget::set_line_join(line_join join)
{
    _commands.emplace_back([=](canvas& canvas) { canvas.set_line_join(join); });
}

void canvas_widget::begin_path()
{
    _commands.emplace_back([=](canvas& canvas) { canvas.begin_path(); });
}

void canvas_widget::close_path()
{
    _commands.emplace_back([=](canvas& canvas) { canvas.close_path(); });
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

void canvas_widget::fill()
{
    _commands.emplace_back([=](canvas& canvas) { canvas.fill(); });
}

void canvas_widget::stroke()
{
    _commands.emplace_back([=](canvas& canvas) { canvas.stroke(); });
}

void canvas_widget::clear()
{
    _commands.clear();
}

void canvas_widget::on_paint(widget_painter& painter)
{
    auto& canvas {painter.get_canvas()};
    auto  guard {canvas.create_guard()};

    canvas.set_scissor(Bounds);
    canvas.translate(Bounds->get_position());
    for (auto& command : _commands) {
        command(canvas);
    }
}

void canvas_widget::on_update(milliseconds /* deltaTime */)
{
}

} // namespace display
