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

} // namespace display
