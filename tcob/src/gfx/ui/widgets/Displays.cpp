// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Displays.hpp"

#include <algorithm>
#include <array>
#include <bitset>
#include <iterator>
#include <span>
#include <utility>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void dot_matrix_display::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    if (left.Colors.size() != right.Colors.size()) { return; }
    for (auto const& [k, v] : left.Colors) {
        if (right.Colors.contains(k)) {
            target.Colors[k] = color::Lerp(v, right.Colors.at(k), step);
        }
    }
}

dot_matrix_display::dot_matrix_display(init const& wi)
    : widget {wi}
{
    Size.Changed.connect([this](auto const&) {
        _isDirty = true;
        request_redraw(this->name() + ": Size changed");
    });
    Dots.Changed.connect([this](auto const&) {
        _isDirty = true;
        request_redraw(this->name() + ": Dots changed");
    });

    Class("dot_matrix_display");
}

void dot_matrix_display::on_draw(widget_painter& painter)
{

    if (Size->Width <= 0 || Size->Height <= 0) { return; }
    apply_style(_style);

    rect_f rect {Bounds()};

    // background
    painter.draw_background_and_border(_style, rect, false);

    scissor_guard const guard {painter, this};

    auto& canvas {painter.canvas()};
    canvas.save();

    f32 const width {rect.width() / Size->Width};
    f32 const height {rect.height() / Size->Height};

    for (auto const& [colorIdx, dots] : _sortedDots) {
        canvas.set_fill_style(_style.Colors.at(colorIdx));
        canvas.begin_path();

        for (auto const& dot : dots) {
            rect_f const dotRect {(dot.X * width) + rect.left(),
                                  (dot.Y * height) + rect.top(),
                                  width, height};

            switch (_style.Type) {
            case dot_type::Disc: canvas.ellipse(dotRect.center(), width / 2, height / 2); break;
            case dot_type::Square: canvas.rect(dotRect); break;
            }
        }

        canvas.fill();
    }

    canvas.restore();
}

void dot_matrix_display::on_update(milliseconds /* deltaTime */)
{
    if (!_isDirty) { return; }
    _isDirty = false;

    _sortedDots.clear();
    for (i32 idx {0}; idx < std::ssize(*Dots); ++idx) {
        _sortedDots[(*Dots)[idx]].emplace_back(idx % Size->Width, idx / Size->Width);
    }
}

////////////////////////////////////////////////////////////

void seven_segment_display::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    target.Size          = length::Lerp(left.Size, right.Size, step);
    target.ActiveColor   = color::Lerp(left.ActiveColor, right.ActiveColor, step);
    target.InactiveColor = color::Lerp(left.InactiveColor, right.InactiveColor, step);
}

seven_segment_display::seven_segment_display(init const& wi)
    : widget {wi}
{
    Class("seven_segment_display");
}

void seven_segment_display::draw_text(string const& text)
{
    _segments.resize(text.size());
    std::ranges::transform(text, _segments.begin(), [this](char const c) { return get_segment(c); });

    request_redraw(this->name() + ": segments changed");
}

void seven_segment_display::draw_segments(std::span<segment const> segments)
{
    _segments.resize(segments.size());
    std::ranges::transform(segments, _segments.begin(), [this](segment const& seg) { return get_segment(seg); });

    request_redraw(this->name() + ": segments changed");
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
}

auto seven_segment_display::get_segment(segment segment) -> std::bitset<7>
{
    return {(static_cast<u32>(segment.A) << 6)
            | (static_cast<u32>(segment.F) << 5)
            | (static_cast<u32>(segment.B) << 4)
            | (static_cast<u32>(segment.G) << 3)
            | (static_cast<u32>(segment.E) << 2)
            | (static_cast<u32>(segment.C) << 1)
            | (static_cast<u32>(segment.D))};
}

void seven_segment_display::on_draw(widget_painter& painter)
{
    apply_style(_style);

    rect_f rect {Bounds()};

    // background
    painter.draw_background_and_border(_style, rect, false);

    scissor_guard const guard {painter, this};

    auto& canvas {painter.canvas()};
    canvas.save();

    f32 const width {_style.Size.calc(rect.width())};
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

    enum class OffsetType {
        None,
        Plus,
        Minus
    };

    static std::array<std::vector<std::pair<usize, OffsetType>>, 7> const segmentDefs {{
        // top horizontal
        {{{0, OffsetType::Plus}, {1, OffsetType::Minus}, {2, OffsetType::Minus}, {3, OffsetType::Plus}}},
        // top left vertical
        {{{0, OffsetType::None}, {3, OffsetType::None}, {5, OffsetType::None}, {6, OffsetType::None}, {4, OffsetType::None}}},
        // top right vertical
        {{{2, OffsetType::None}, {1, OffsetType::None}, {17, OffsetType::None}, {15, OffsetType::None}, {16, OffsetType::None}}},
        // middle horizontal
        {{{6, OffsetType::Plus}, {5, OffsetType::Plus}, {16, OffsetType::Minus}, {15, OffsetType::Minus}, {13, OffsetType::Minus}, {8, OffsetType::Plus}}},
        // bottom left vertical
        {{{6, OffsetType::None}, {8, OffsetType::None}, {10, OffsetType::None}, {9, OffsetType::None}, {7, OffsetType::None}}},
        // bottom right vertical
        {{{15, OffsetType::None}, {14, OffsetType::None}, {12, OffsetType::None}, {11, OffsetType::None}, {13, OffsetType::None}}},
        // bottom horizontal
        {{{10, OffsetType::Plus}, {11, OffsetType::Minus}, {12, OffsetType::Minus}, {9, OffsetType::Plus}}},
    }};

    for (auto const segments : _segments) {
        for (usize i {0}; i < 7; i++) {
            bool const active {segments[6 - i]};
            if (!active && _style.InactiveColor.A == 0) { continue; }
            canvas.set_fill_style(active ? _style.ActiveColor : _style.InactiveColor);

            std::vector<point_f> polyline;
            polyline.reserve(segmentDefs[i].size());
            for (auto const& [idx, offType] : segmentDefs[i]) {
                point_f pt {points[idx] + offset};
                if (offType == OffsetType::Plus) {
                    pt = pt + lineOffset;
                } else if (offType == OffsetType::Minus) {
                    pt = pt - lineOffset;
                }
                polyline.push_back(pt);
            }
            canvas.fill_polyline(polyline);
        }
        offset.X += width + thickness;
    }

    canvas.restore();
}

void seven_segment_display::on_update(milliseconds /* deltaTime */)
{
}

////////////////////////////////////////////////////////////

color_picker::color_picker(init const& wi)
    : widget {wi}
{
    BaseHue.Changed.connect([this](auto const&) { request_redraw(this->name() + ": BaseHue changed"); });

    Class("color_picker");
}

void color_picker::on_draw(widget_painter& painter)
{

    auto& canvas {painter.canvas()};
    auto  guard {canvas.create_guard()};

    auto const bounds {Bounds()};

    canvas.set_scissor(bounds);
    canvas.translate(bounds.Position);

    // color gradient
    size_f const sizeColor {bounds.width() * 0.9f, bounds.height()};

    canvas.begin_path();
    canvas.rect({point_f::Zero, sizeColor});

    canvas.set_fill_style(colors::White);
    canvas.fill();

    color const baseColor {color::FromHSVA({BaseHue(), 1.0, 1.0f})};
    canvas.set_fill_style(canvas.create_linear_gradient(
        {0, 0}, {sizeColor.Width, 0},
        {color {baseColor.R, baseColor.G, baseColor.B, 0}, color {baseColor.R, baseColor.G, baseColor.B, 255}}));
    canvas.fill();
    canvas.set_fill_style(canvas.create_linear_gradient(
        {0, 0}, {0, sizeColor.Height},
        {color {0, 0, 0, 0}, color {0, 0, 0, 255}}));
    canvas.fill();

    // hue gradient
    size_f const sizeHue {bounds.width() * 0.1f, bounds.height()};

    canvas.begin_path();
    canvas.rect({{sizeColor.Width, 0}, sizeHue});
    canvas.set_fill_style(canvas.create_linear_gradient({0, 0}, {0, sizeHue.Height}, GetGradient()));
    canvas.fill();

    if (_selectedColorPos.X > -1) {
        canvas.begin_path();
        canvas.circle(_selectedColorPos, 5);
        canvas.set_stroke_style(colors::White);
        canvas.set_stroke_width(2);
        canvas.stroke();
    }

    if (_selectedHuePos > -1) {
        canvas.begin_path();
        canvas.move_to({sizeColor.Width, _selectedHuePos});
        canvas.line_to({sizeColor.Width + sizeHue.Width, _selectedHuePos});
        canvas.set_stroke_style(colors::White);
        canvas.set_stroke_width(2);
        canvas.stroke();
    }
}

void color_picker::on_mouse_down(input::mouse::button_event const& ev)
{
    auto const mp {global_to_parent(ev.Position)};
    if (Bounds->contains(mp)) {
        f32 const s {(mp.X - Bounds->left()) / (Bounds->width() * 0.9f)};
        f32 const v {(mp.Y - Bounds->top()) / Bounds->height()};
        if (s > 1.0f) {
            auto const col {GetGradient().colors().at(static_cast<i32>(255 * v))};
            BaseHue           = col.to_hsv().Hue;
            _selectedHuePos   = mp.Y - Bounds->Position.Y;
            _selectedColorPos = {-1, -1};
        } else {
            SelectedColor     = color::FromHSVA({BaseHue(), s, 1 - v});
            _selectedColorPos = mp - Bounds->Position;
            request_redraw(this->name() + ": SelectedColor changed");
        }
    }
}

void color_picker::on_update(milliseconds /* deltaTime */)
{
}

auto color_picker::GetGradient() -> gfx::color_gradient const&
{
    static std::array<gfx::color_stop, 7> colorStops {{{0.f, colors::Red}, {1 / 6.f, colors::Orange}, {2 / 6.f, colors::Yellow}, {3 / 6.f, colors::Green}, {4 / 6.f, colors::Blue}, {5 / 6.f, colors::Indigo}, {1.f, colors::Violet}}};
    static gfx::color_gradient            grad {colorStops};
    return grad;
}

} // namespace display
