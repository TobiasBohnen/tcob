// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/WidgetPainter.hpp"

#include "tcob/core/Common.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {

widget_painter::widget_painter(canvas& canvas)
    : _canvas {canvas}
{
}

void widget_painter::begin(f32 alpha)
{
    _canvas.save();
    _canvas.set_global_alpha(alpha);
}

void widget_painter::begin(f32 alpha, transform const& xform)
{
    begin(alpha);
    _canvas.set_transform(xform);
}

void widget_painter::end()
{
    _canvas.restore();
}

void widget_painter::push_scissor(rect_f const& globalScissor)
{
    if (_scissorStack.empty()) {
        _scissorStack.push(globalScissor);
    } else {
        _scissorStack.push(globalScissor.as_intersection_with(_scissorStack.top()));
    }

    _canvas.set_scissor(_scissorStack.top(), false);
}

void widget_painter::pop_scissor()
{
    _scissorStack.pop();
}

////////////////////////////////////////////////////////////

void widget_painter::draw_background_and_border(background_style const& style, rect_f& refRect, bool isCircle)
{
    // add margin
    refRect -= style.Margin;

    if (isCircle) {
        draw_shadow(style.DropShadow, refRect, isCircle, style.Border);
        draw_bordered_circle(refRect, style.Background, style.Border);
    } else {
        draw_shadow(style.DropShadow, refRect, isCircle, style.Border);
        draw_bordered_rect(refRect, style.Background, style.Border);
    }

    // add padding
    refRect -= style.Padding;

    // add border
    refRect -= style.Border.get_thickness();
}

void widget_painter::draw_bordered_rect(rect_f const& rect, ui_paint const& back, element::border const& borderStyle)
{
    auto const guard {_canvas.create_guard()};

    if (auto const* np {std::get_if<nine_patch>(&back)}) {
        draw_nine_patch(*np, rect, borderStyle);
    } else {
        // background
        f32 const borderRadius {borderStyle.Radius.calc(rect.Width)};
        _canvas.set_fill_style(get_paint(back, rect));
        _canvas.begin_path();
        _canvas.rounded_rect(rect, borderRadius);
        _canvas.fill();

        // border
        f32 const borderSize {borderStyle.Size.calc(rect.Width)};
        draw_border(rect, borderStyle, borderSize, borderRadius);
    }
}

void widget_painter::draw_bordered_circle(rect_f const& rect, ui_paint const& back, element::border const& borderStyle)
{
    auto const guard {_canvas.create_guard()};

    if (auto const* np {std::get_if<nine_patch>(&back)}) {
        draw_nine_patch(*np, rect, borderStyle);
    } else {
        // background
        _canvas.set_fill_style(get_paint(back, rect));
        f32 const r {std::min(rect.Height, rect.Width) / 2};
        _canvas.begin_path();
        _canvas.circle(rect.get_center(), r);
        _canvas.fill();

        // border
        f32 const borderSize {borderStyle.Size.calc(rect.Width)};
        draw_border({rect.X + (rect.Width / 2 - r), rect.Y + (rect.Height / 2 - r), r * 2, r * 2}, borderStyle, borderSize, r);
    }
}

void widget_painter::draw_nine_patch(nine_patch const& np, rect_f const& rect, element::border const& borderStyle)
{
    _canvas.set_fill_style(colors::White);
    f32 const    borderSize {borderStyle.Size.calc(rect.Width)};
    rect_f const center {rect.X + borderSize, rect.Y + borderSize, rect.Width - borderSize * 2, rect.Height - borderSize * 2};
    _canvas.draw_nine_patch(np.Texture.get_obj(), np.Region, rect, center, np.UV);
}

void widget_painter::draw_border(rect_f const& rect, element::border const& borderStyle, f32 borderSize, f32 borderRadius)
{
    if (borderSize <= 0.0f) {
        return;
    }

    switch (borderStyle.Type) {
    case element::border::type::Solid: {
        _canvas.set_stroke_style(get_paint(borderStyle.Background, rect));
        _canvas.set_stroke_width(borderSize);
        _canvas.begin_path();
        _canvas.rounded_rect(rect, borderRadius);
        _canvas.stroke();
    } break;
    case element::border::type::Double: {
        _canvas.set_stroke_style(get_paint(borderStyle.Background, rect));
        f32 const dborderSize {borderSize / 3};
        _canvas.set_stroke_width(dborderSize);
        _canvas.begin_path();
        _canvas.rounded_rect({rect.X - dborderSize * 2, rect.Y - dborderSize * 2, rect.Width + dborderSize * 4, rect.Height + dborderSize * 4}, borderRadius);
        _canvas.rounded_rect(rect, borderRadius);
        _canvas.stroke();

    } break;
    case element::border::type::Hidden:
        break;
    }
}

void widget_painter::draw_text(element::text const& style, rect_f const& refRect, utf8_string const& text)
{
    if (text.empty()) { return; }

    draw_text(style, refRect, format_text(style, refRect, text));
}

void widget_painter::draw_text(element::text const& style, rect_f const& refRect, text_formatter::result const& text)
{
    auto const guard {_canvas.create_guard()};

    _canvas.set_font(text.Font);
    _canvas.set_text_halign(style.Alignment.Horizontal);
    _canvas.set_text_valign(style.Alignment.Vertical);

    // shadow
    if (style.Shadow.Color.A != 0) {
        point_f const dropShadowOffset {style.Shadow.OffsetX.calc(refRect.Width),
                                        style.Shadow.OffsetY.calc(refRect.Height)};

        _canvas.set_fill_style(get_paint(style.Shadow.Color, refRect));
        rect_f shadowRect {refRect};
        shadowRect.X += dropShadowOffset.X;
        shadowRect.Y += dropShadowOffset.Y;
        _canvas.draw_textbox(shadowRect.get_position(), text);
    }

    // text
    _canvas.set_fill_style(get_paint(style.Color, refRect));
    _canvas.draw_textbox(refRect.get_position(), text);

    // deco
    auto const& deco {style.Decoration};
    if ((deco.Line.LineThrough || deco.Line.Overline || deco.Line.Underline) && deco.Color.A > 0) {
        f32 const strokeWidth {deco.Size.calc(refRect.Height)};

        for (auto const& token : text.Tokens) {
            if (token.Quads.empty()) {
                continue;
            }

            rect_f first {token.Quads[0].Rect};
            rect_f last {};

            auto const drawDeco {[&]() {
                auto const drawLine {[&](point_f p0, point_f p1, point_f offset) {
                    switch (deco.Style) {
                    case text_decoration::style::Solid:
                    case text_decoration::style::Double:
                        _canvas.set_stroke_width(strokeWidth);
                        _canvas.set_stroke_style(deco.Color);
                        _canvas.stroke_line(p0 + offset, p1 + offset);
                        break;
                    case text_decoration::style::Dotted:
                        _canvas.set_fill_style(deco.Color);
                        _canvas.begin_path();
                        _canvas.dotted_line(p0 + offset, p1 + offset, strokeWidth, std::max(1, static_cast<i32>((p1.X - p0.X) / (strokeWidth * 2.5f))));
                        _canvas.fill();
                        break;
                    case text_decoration::style::Dashed:
                        _canvas.set_stroke_width(strokeWidth);
                        _canvas.set_stroke_style(deco.Color);
                        _canvas.stroke_dashed_line(p0 + offset, p1 + offset, std::max(1, static_cast<i32>((p1.X - p0.X) / (strokeWidth * 2.5f))));
                        break;
                    case text_decoration::style::Wavy:
                        _canvas.set_stroke_width(strokeWidth);
                        _canvas.set_stroke_style(deco.Color);
                        _canvas.begin_path();
                        _canvas.wavy_line(p0 + offset, p1 + offset, strokeWidth * 1.25f, 0.5f);
                        _canvas.stroke();
                        break;
                    }
                }};

                point_f offset {refRect.top_left()};

                if (deco.Line.Underline) {
                    point_f const p0 {first.bottom_left()};
                    point_f const p1 {last.bottom_right()};
                    offset += point_f {0, strokeWidth / 2};
                    drawLine(p0, p1, offset);
                    if (deco.Style == text_decoration::style::Double) {
                        offset += point_f {0, strokeWidth * 2};
                        drawLine(p0, p1, offset);
                    }
                }
                if (deco.Line.Overline) {
                    point_f const p0 {first.top_left()};
                    point_f const p1 {last.top_right()};
                    offset -= point_f {0, strokeWidth / 2};
                    drawLine(p0, p1, offset);
                    if (deco.Style == text_decoration::style::Double) {
                        offset -= point_f {0, strokeWidth * 2};
                        drawLine(p0, p1, offset);
                    }
                }
                if (deco.Line.LineThrough) {
                    point_f const p0 {first.top_left()};
                    point_f const p1 {last.top_right()};
                    offset += point_f {0, first.Height / 3 * 2};
                    drawLine(p0, p1, offset);
                    if (deco.Style == text_decoration::style::Double) {
                        offset -= point_f {0, strokeWidth * 2};
                        drawLine(p0, p1, offset);
                    }
                }
            }};

            for (auto const& quad : token.Quads) {
                if (first.bottom() != quad.Rect.bottom()) {
                    drawDeco();
                    first = quad.Rect;
                    continue;
                }
                last = quad.Rect;
            }

            drawDeco();
        }
    }
}

void widget_painter::draw_tick(element::tick const& style, rect_f const& refRect)
{
    auto const guard {_canvas.create_guard()};

    if (auto const* np {std::get_if<nine_patch>(&style.Foreground)}) {
        draw_nine_patch(*np, refRect, {});
    } else {
        switch (style.Type) {
        case element::tick::type::Checkmark: {
            f32 const width {style.Size.calc(std::min(refRect.Height, refRect.Width)) / 4};
            _canvas.set_stroke_width(width);
            _canvas.set_stroke_style(get_paint(style.Foreground, refRect));
            _canvas.begin_path();
            _canvas.move_to({refRect.left(), refRect.get_center().Y});
            _canvas.line_to({refRect.get_center().X, refRect.bottom()});
            _canvas.line_to(refRect.top_right());
            _canvas.stroke();
        } break;
        case element::tick::type::Cross: {
            f32 const width {style.Size.calc(std::min(refRect.Height, refRect.Width)) / 4};
            _canvas.set_stroke_width(width);
            _canvas.set_stroke_style(get_paint(style.Foreground, refRect));
            _canvas.begin_path();
            _canvas.move_to(refRect.top_left());
            _canvas.line_to(refRect.bottom_right());
            _canvas.move_to(refRect.top_right());
            _canvas.line_to(refRect.bottom_left());
            _canvas.stroke();
        } break;
        case element::tick::type::Circle: {
            f32 const width {style.Size.calc(std::min(refRect.Height, refRect.Width)) / 3};
            _canvas.set_stroke_width(width);
            _canvas.set_stroke_style(get_paint(style.Foreground, refRect));
            _canvas.begin_path();
            _canvas.circle(refRect.get_center(), width);
            _canvas.stroke();
        } break;
        case element::tick::type::Disc: {
            f32 const width {style.Size.calc(std::min(refRect.Height, refRect.Width)) / 2};
            _canvas.set_fill_style(get_paint(style.Foreground, refRect));
            _canvas.begin_path();
            _canvas.circle(refRect.get_center(), width);
            _canvas.fill();
        } break;
        case element::tick::type::Rect: {
            rect_f const newRect {refRect.X + style.Size.calc(refRect.Width),
                                  refRect.Y + style.Size.calc(refRect.Height),
                                  refRect.Width - (style.Size.calc(refRect.Width) + style.Size.calc(refRect.Width)),
                                  refRect.Height - (style.Size.calc(refRect.Height) + style.Size.calc(refRect.Height))};
            _canvas.set_fill_style(get_paint(style.Foreground, newRect));
            _canvas.begin_path();
            _canvas.rect(newRect);
            _canvas.fill();
        } break;
        case element::tick::type::Square: {
            f32 const    width {style.Size.calc(std::min(refRect.Height, refRect.Width))};
            rect_f const newRect {refRect.get_center() - point_f {width, width} / 2, {width, width}};
            _canvas.set_fill_style(get_paint(style.Foreground, newRect));
            _canvas.begin_path();
            _canvas.rect(newRect);
            _canvas.fill();
        } break;
        case element::tick::type::None:
            break;
        }
    }
}

auto widget_painter::draw_bar(element::bar const& style, rect_f const& refRect, element::bar::context const& barCtx) -> rect_f
{
    rect_f retValue {style.calc(refRect, barCtx.Orientation, barCtx.Position)};
    rect_f lowRech {retValue};

    if (style.Type == element::bar::type::Continuous) {
        // Higher
        draw_bordered_rect(retValue, style.HigherBackground, style.Border);
        if (style.HigherBackground != style.LowerBackground) {
            // Lower
            switch (barCtx.Orientation) {
            case orientation::Horizontal:
                lowRech.Width *= (barCtx.Inverted ? 1.0f - barCtx.Fraction : barCtx.Fraction);
                break;
            case orientation::Vertical:
                lowRech.Height *= (barCtx.Inverted ? 1.0f - barCtx.Fraction : barCtx.Fraction);
                lowRech.Y += retValue.Height - lowRech.Height;
                break;
            }
            if (lowRech.Height > 0 && lowRech.Width > 0) {
                draw_bordered_rect(lowRech, style.LowerBackground, style.Border);
            }
        }
    } else if (style.Type == element::bar::type::Blocks) {
        for (i32 i {0}; i < barCtx.BlockCount; ++i) {
            rect_f   blockRect {retValue};
            ui_paint back;

            switch (barCtx.Orientation) {
            case orientation::Horizontal:
                blockRect.Width /= barCtx.BlockCount;
                blockRect.X += blockRect.Width * i;
                back = i / static_cast<f32>(barCtx.BlockCount) < (barCtx.Inverted ? 1.0f - barCtx.Fraction : barCtx.Fraction)
                    ? style.LowerBackground
                    : style.HigherBackground;
                break;
            case orientation::Vertical:
                blockRect.Height /= barCtx.BlockCount;
                blockRect.Y += blockRect.Height * i;
                back = i / static_cast<f32>(barCtx.BlockCount) < (!barCtx.Inverted ? 1.0f - barCtx.Fraction : barCtx.Fraction)
                    ? style.HigherBackground
                    : style.LowerBackground;
                break;
            }

            draw_bordered_rect(blockRect, back, style.Border);
        }
    }

    return retValue;
}

auto widget_painter::draw_thumb(element::thumb const& style, rect_f const& refRect, element::thumb::context const& thumbCtx) -> rect_f
{
    rect_f retValue {style.calc(refRect, thumbCtx)};

    if (style.Type == element::thumb::type::Rect) {
        draw_bordered_rect(retValue, style.Background, style.Border);
    } else if (style.Type == element::thumb::type::Disc) {
        draw_bordered_circle(retValue, style.Background, style.Border);
    }

    return retValue;
}

void widget_painter::draw_caret(element::caret const& style, rect_f const& refRect, point_f offset)
{
    rect_f rect {refRect};
    rect.Width = style.Width.calc(rect.Width);
    rect.X += offset.X;
    rect.Y += offset.Y;

    draw_bordered_rect(rect, style.Color, {});
}

auto widget_painter::draw_scrollbar(element::scrollbar const& style, element::thumb const& thumbStyle, rect_f const& refRect, element::bar::context const& barCtx) -> element::scrollbar::rects
{
    element::scrollbar::rects retValue;

    retValue.Bar   = draw_bar(style.Bar, refRect, barCtx);
    retValue.Thumb = draw_thumb(thumbStyle, retValue.Bar, {.Orientation = barCtx.Orientation, .Inverted = barCtx.Inverted, .Fraction = barCtx.Fraction});

    return retValue;
}

void widget_painter::draw_nav_arrow(element::nav_arrow const& style, rect_f const& refRect)
{
    auto const guard {_canvas.create_guard()};

    rect_f const navRect {style.calc(refRect)};

    rect_f decRect {navRect};
    draw_bordered_rect(decRect, style.DecBackground, style.Border);

    switch (style.Type) {
    case element::nav_arrow::type::Triangle: {
        _canvas.set_fill_style(get_paint(style.Foreground, navRect));
        _canvas.begin_path();
        _canvas.triangle(
            {navRect.left() + 2, navRect.Y + 4},
            {navRect.get_center().X, navRect.bottom() - 4},
            {navRect.right() - 2, navRect.Y + 4});
        _canvas.fill();
    } break;
    case element::nav_arrow::type::None:
        break;
    }
}

void widget_painter::draw_nav_arrows(element::nav_arrow const& incStyle, element::nav_arrow const& decStyle, rect_f const& refRect)
{
    auto const guard {_canvas.create_guard()};

    {
        rect_f const navRect {decStyle.calc(refRect)};

        rect_f decRect {navRect};
        decRect.Height /= 2;
        decRect.Y += decRect.Height;
        draw_bordered_rect(decRect, decStyle.DecBackground, decStyle.Border);

        switch (decStyle.Type) {
        case element::nav_arrow::type::Triangle: {
            point_f const center {navRect.get_center()};
            _canvas.set_fill_style(get_paint(decStyle.Foreground, navRect));
            _canvas.begin_path();
            _canvas.triangle(
                {navRect.left() + 2, center.Y + 4},
                {center.X, navRect.bottom() - 4},
                {navRect.right() - 2, center.Y + 4});
            _canvas.fill();
        } break;
        case element::nav_arrow::type::None:
            break;
        }
    }

    {
        rect_f const navRect {incStyle.calc(refRect)};

        rect_f incRect {navRect};
        incRect.Height /= 2;
        draw_bordered_rect(incRect, incStyle.IncBackground, incStyle.Border);

        switch (incStyle.Type) {
        case element::nav_arrow::type::Triangle: {
            point_f const center {navRect.get_center()};
            _canvas.set_fill_style(get_paint(incStyle.Foreground, navRect));
            _canvas.begin_path();
            _canvas.triangle(
                {navRect.left() + 2, center.Y - 4},
                {center.X, navRect.top() + 4},
                {navRect.right() - 2, center.Y - 4});
            _canvas.fill();
        } break;
        case element::nav_arrow::type::None:
            break;
        }
    }
}

void widget_painter::draw_item(element::item const& style, rect_f const& rect, utf8_string const& text)
{
    rect_f itemRect {rect};
    draw_bordered_rect(itemRect, style.Background, style.Border);

    if (style.Text.Font) {
        itemRect -= style.Padding;
        itemRect -= style.Border.get_thickness();
        draw_text(style.Text, itemRect, text);
    }
}

void widget_painter::draw_shadow(element::shadow const& style, rect_f const& refRect, bool isCircle, element::border const& borderStyle)
{
    if (style.Color.A != 0) {
        point_f const dropShadowOffset {style.OffsetX.calc(refRect.Width),
                                        style.OffsetY.calc(refRect.Height)};

        rect_f shadowRect {refRect};
        shadowRect.X += dropShadowOffset.X;
        shadowRect.Y += dropShadowOffset.Y;

        if (isCircle) {
            draw_bordered_circle(shadowRect, style.Color, {.Radius = borderStyle.Radius});
        } else {
            draw_bordered_rect(shadowRect, style.Color, {.Radius = borderStyle.Radius});
        }
    }
}

auto widget_painter::get_canvas() -> canvas&
{
    return _canvas;
}

auto widget_painter::format_text(element::text const& style, rect_f const& rect, utf8_string_view text) -> text_formatter::result
{
    auto const tt {transform_text(style.Transform, text)};
    return format_text(style, rect, tt, style.calc_font_size(rect), true);
}

////////////////////////////////////////////////////////////

auto widget_painter::format_text(element::text const& style, rect_f const& rect, utf8_string_view text, u32 fontSize, bool resize) -> text_formatter::result
{
    auto const guard {_canvas.create_guard()};

    auto* const font {style.Font->get_font(style.Style, fontSize).get_obj()};

    _canvas.set_font(font);
    _canvas.set_text_halign(style.Alignment.Horizontal);
    _canvas.set_text_valign(style.Alignment.Vertical);

    auto const rectSize {rect.get_size()};

    f32 scale {1.0f};

    if (resize && style.AutoSize != element::text::auto_size_mode::Never) {
        auto const textSize {_canvas.measure_text(-1, text)};

        bool const shouldShrink {(style.AutoSize == element::text::auto_size_mode::Always || style.AutoSize == element::text::auto_size_mode::OnlyShrink)
                                 && (textSize.Width > rectSize.Width || textSize.Height > rectSize.Height)};

        bool const shouldGrow {(style.AutoSize == element::text::auto_size_mode::Always || style.AutoSize == element::text::auto_size_mode::OnlyGrow)
                               && (textSize.Width < rectSize.Width && textSize.Height < rectSize.Height)};

        if (shouldShrink || shouldGrow) {
            scale = std::min(std::floor(rectSize.Width) / textSize.Width, std::floor(rectSize.Height) / textSize.Height);
        }
    }

    return _canvas.format_text(rectSize, text, scale);
}

auto widget_painter::transform_text(element::text::transform xform, utf8_string_view text) const -> utf8_string
{
    utf8_string retValue {text};
    switch (xform) {
    case element::text::transform::Capitalize: {
        bool newWord {true};
        for (auto& c : retValue) {
            if (std::isspace(c)) {
                newWord = true;
            } else if (newWord) {
                c       = static_cast<char>(std::toupper(c));
                newWord = false;
            }
        }
    } break;
    case element::text::transform::Lowercase:
        for (auto& c : retValue) { c = static_cast<char>(std::tolower(c)); }
        break;
    case element::text::transform::Uppercase:
        for (auto& c : retValue) { c = static_cast<char>(std::toupper(c)); }
        break;
    default:
        break;
    }
    return retValue;
}

auto widget_painter::get_paint(ui_paint const& p, rect_f const& rect) -> canvas_paint
{
    return std::visit(
        overloaded {
            [&](color const& arg) -> canvas_paint {
                return canvas_paint {
                    .Feather = 1.0f,
                    .Color   = arg,
                };
            },
            [&](linear_gradient const& arg) -> canvas_paint {
                degree_f const angle {arg.Angle + degree_f {90}};
                point_f const  p0 {rect.find_edge(angle)};
                point_f const  p1 {rect.find_edge(angle - degree_f {180})};
                return _canvas.create_linear_gradient(p0, p1, arg.Colors);
            },
            [&](radial_gradient const& arg) -> canvas_paint {
                return _canvas.create_radial_gradient(
                    rect.get_center(),
                    arg.InnerRadius.calc(rect.Width), arg.OuterRadius.calc(rect.Width),
                    arg.Scale, arg.Colors);
            },
            [&](box_gradient const& arg) -> canvas_paint {
                return _canvas.create_box_gradient(
                    rect,
                    arg.Radius.calc(rect.Width), arg.Feather.calc(rect.Width), arg.Colors);
            },
            [&](image_pattern const& arg) -> canvas_paint {
                return _canvas.create_image_pattern(
                    rect.top_left(),
                    arg.Stretch ? rect.get_size() : size_f {arg.Texture->get_size()},
                    degree_f {0}, arg.Texture.get_obj(), 1.0f);
            },
            [&](nine_patch const&) -> canvas_paint {
                return {};
            },
        },
        p);
}

////////////////////////////////////////////////////////////

scissor_guard::scissor_guard(widget_painter& painter, widget* w)
    : _painter {painter}
{
    rect_f bounds {w->get_global_content_bounds()};

    if (auto const* form {w->get_form()}) {
        point_f const off {form->Bounds->get_position()};
        bounds.X -= off.X;
        bounds.Y -= off.Y;
    }
    painter.push_scissor(bounds);
}

scissor_guard::~scissor_guard()
{
    _painter.pop_scissor();
}

} // namespace ui
