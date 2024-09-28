// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Displays.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

dot_matrix_display::dot_matrix_display(init const& wi)
    : widget {wi}
{
    Size.Changed.connect([&](auto const&) {
        _isDirty = true;
        force_redraw(get_name() + ": Size changed");
    });
    Dots.Changed.connect([&](auto const&) {
        _isDirty = true;
        force_redraw(get_name() + ": Dots changed");
    });

    Class("dot_matrix_display");
}

void dot_matrix_display::on_paint(widget_painter& painter)
{
    if (Size->Width <= 0 || Size->Height <= 0) {
        return;
    }

    if (auto const* style {get_style<dot_matrix_display::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        auto& canvas {painter.get_canvas()};
        canvas.save();

        f32 const width {rect.Width / Size->Width};
        f32 const height {rect.Height / Size->Height};

        for (auto const& [colorIdx, dots] : _sortedDots) {
            canvas.set_fill_style(style->Dot.Colors.at(colorIdx));
            canvas.begin_path();

            for (auto const& dot : dots) {
                rect_f const dotRect {(dot.X * width) + rect.left(),
                                      (dot.Y * height) + rect.top(),
                                      width, height};

                switch (style->Dot.Type) {
                case dot::type::Disc: {
                    canvas.circle(dotRect.get_center(), width / 2);
                } break;
                case dot::type::Square: {
                    canvas.rect(dotRect);
                } break;
                }
            }

            canvas.fill();
        }

        canvas.restore();
    }
}

void dot_matrix_display::on_update(milliseconds /* deltaTime */)
{
    if (_isDirty) {
        _sortedDots.clear();
        for (i32 idx {0}; idx < std::ssize(*Dots); ++idx) {
            point_i const dotPoint {idx % Size->Width, idx / Size->Width};
            _sortedDots[(*Dots)[idx]].push_back(dotPoint);
        }
        _isDirty = false;
    }
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
    if (auto* const style {get_style<seven_segment_display::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        auto& canvas {painter.get_canvas()};
        canvas.save();

        f32 const width {style->Segment.Size.calc(rect.Width)};
        f32 const thickness {width / 4};

        point_f       offset {rect.top_left()};
        point_f const lineOffset {1, 0};

        std::array<point_f, 18> const points {{
            {0, 0},                                       // 01
            {width, 0},                                   // 02
            {width - thickness, thickness},               // 03
            {thickness, thickness},                       // 04

            {0, width - (thickness / 2)},                 // 05
            {thickness, width - (thickness / 2)},         // 06
            {thickness / 2, width},                       // 07

            {0, width + (thickness / 2)},                 // 08
            {thickness, width + (thickness / 2)},         // 09
            {0, width * 2},                               // 10
            {thickness, (width * 2) - thickness},         // 11

            {width - thickness, (width * 2) - thickness}, // 12
            {width, width * 2},                           // 13

            {width - thickness, width + (thickness / 2)}, // 14
            {width, width + (thickness / 2)},             // 15
            {width - (thickness / 2), width},             // 16

            {width - thickness, width - (thickness / 2)}, // 17
            {width, width - (thickness / 2)},             // 18
        }};

        for (auto const c : Text()) {
            auto const segments {get_segment(c)};

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
                    canvas.fill_lines({{points[0] + offset + lineOffset,
                                        points[1] + offset - lineOffset,
                                        points[2] + offset - lineOffset,
                                        points[3] + offset + lineOffset}});
                    break;
                case 1:
                    canvas.fill_lines({{points[0] + offset,
                                        points[3] + offset,
                                        points[5] + offset,
                                        points[6] + offset,
                                        points[4] + offset}});
                    break;
                case 2:
                    canvas.fill_lines({{points[2] + offset,
                                        points[1] + offset,
                                        points[17] + offset,
                                        points[15] + offset,
                                        points[16] + offset}});
                    break;
                case 3:
                    canvas.fill_lines({{points[6] + offset + lineOffset,
                                        points[5] + offset + lineOffset,
                                        points[16] + offset - lineOffset,
                                        points[15] + offset - lineOffset,
                                        points[13] + offset - lineOffset,
                                        points[8] + offset + lineOffset}});
                    break;
                case 4:
                    canvas.fill_lines({{points[6] + offset,
                                        points[8] + offset,
                                        points[10] + offset,
                                        points[9] + offset,
                                        points[7] + offset}});
                    break;
                case 5:
                    canvas.fill_lines({{points[15] + offset, points[14] + offset,
                                        points[12] + offset,
                                        points[11] + offset,
                                        points[13] + offset}});
                    break;
                case 6:
                    canvas.fill_lines({{points[10] + offset + lineOffset,
                                        points[11] + offset - lineOffset,
                                        points[12] + offset - lineOffset,
                                        points[9] + offset + lineOffset}});
                    break;
                }
            }
            offset.X += width + thickness;
        }

        canvas.restore();
    }
}

void seven_segment_display::on_update(milliseconds /* deltaTime */)
{
}

////////////////////////////////////////////////////////////

color_picker::color_picker(init const& wi)
    : widget {wi}
{
    BaseHue.Changed.connect([&](auto const&) { force_redraw(get_name() + ": BaseHue changed"); });
    BaseHue(0);

    Class("color_picker");
}

void color_picker::on_paint(widget_painter& painter)
{
    auto& canvas {painter.get_canvas()};
    auto  guard {canvas.create_guard()};

    auto const bounds {Bounds()};

    canvas.set_scissor(bounds);
    canvas.translate(bounds.get_position());

    // color gradient
    size_f const rect0 {bounds.Width * 0.9f, bounds.Height};

    canvas.begin_path();
    canvas.rect({point_f::Zero, rect0});

    canvas.set_fill_style(colors::White);
    canvas.fill();

    color const baseColor {color::FromHSVA({BaseHue, 1.0, 1.0f})};
    canvas.set_fill_style(canvas.create_linear_gradient(
        {0, 0}, {rect0.Width, 0},
        {color {baseColor.R, baseColor.G, baseColor.B, 0}, color {baseColor.R, baseColor.G, baseColor.B, 255}}));
    canvas.fill();
    canvas.set_fill_style(canvas.create_linear_gradient(
        {0, 0}, {0, rect0.Height},
        {color {0, 0, 0, 0}, color {0, 0, 0, 255}}));
    canvas.fill();

    // hue gradient
    size_f const rect1 {bounds.Width * 0.1f, bounds.Height};

    canvas.begin_path();
    canvas.rect({{rect0.Width, 0}, rect1});
    canvas.set_fill_style(canvas.create_linear_gradient({0, 0}, {0, rect1.Height}, GetGradient()));
    canvas.fill();
}

void color_picker::on_mouse_down(input::mouse::button_event const& ev)
{
    rect_f const rect {get_global_content_bounds()};
    if (rect.contains(ev.Position)) {
        f32 const s {(ev.Position.X - rect.X) / (rect.Width * 0.9f)};
        f32 const v {(ev.Position.Y - rect.Y) / rect.Height};
        if (s > 1.0f) {
            auto const col {GetGradient().get_colors().at(static_cast<i32>(255 * v))};
            BaseHue = col.to_hsv().Hue;
        } else {
            SelectedColor = color::FromHSVA({BaseHue, s, 1 - v});
        }
    }
}

void color_picker::on_update(milliseconds /* deltaTime */)
{
}

auto color_picker::GetGradient() -> color_gradient const&
{
    static std::array<color_stop, 7> colorStops {{{0.f, colors::Red}, {1 / 6.f, colors::Orange}, {2 / 6.f, colors::Yellow}, {3 / 6.f, colors::Green}, {4 / 6.f, colors::Blue}, {5 / 6.f, colors::Indigo}, {1.f, colors::Violet}}};
    static color_gradient            grad {colorStops};
    return grad;
}

} // namespace display
