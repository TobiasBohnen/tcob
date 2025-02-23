// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/WidgetPainter.hpp"

#include "tcob/core/Common.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {

widget_painter::widget_painter(gfx::canvas& canvas)
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

void widget_painter::draw_background_and_border(widget_style const& style, rect_f& rect, bool isCircle)
{
    // add margin
    rect -= style.Margin;

    if (isCircle) {
        draw_shadow(style.DropShadow, rect, isCircle, style.Border);
        draw_bordered_circle(rect, style.Background, style.Border);
    } else {
        draw_shadow(style.DropShadow, rect, isCircle, style.Border);
        draw_bordered_rect(rect, style.Background, style.Border);
    }

    // add padding
    rect -= style.Padding;

    // add border
    rect -= style.Border.thickness();
}

void widget_painter::draw_bordered_rect(rect_f const& rect, ui_paint const& back, element::border const& borderStyle)
{
    auto const guard {_canvas.create_guard()};

    if (auto const* np {std::get_if<nine_patch>(&back)}) {
        draw_nine_patch(*np, rect, borderStyle);
    } else {
        // background
        f32 const borderRadius {borderStyle.Radius.calc(rect.width())};
        _canvas.set_fill_style(get_paint(back, rect));
        _canvas.begin_path();
        _canvas.rounded_rect(rect, borderRadius);
        _canvas.fill();

        // border
        f32 const borderSize {borderStyle.Size.calc(rect.width())};
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
        f32 const r {std::min(rect.height(), rect.width()) / 2};
        _canvas.begin_path();
        _canvas.circle(rect.center(), r);
        _canvas.fill();

        // border
        f32 const borderSize {borderStyle.Size.calc(rect.width())};
        draw_border({rect.left() + (rect.width() / 2 - r), rect.top() + (rect.height() / 2 - r), r * 2, r * 2}, borderStyle, borderSize, r);
    }
}

void widget_painter::draw_nine_patch(nine_patch const& np, rect_f const& rect, element::border const& borderStyle)
{
    _canvas.set_fill_style(colors::White);
    f32 const    borderSize {borderStyle.Size.calc(rect.width())};
    rect_f const center {rect.left() + borderSize, rect.top() + borderSize, rect.width() - (borderSize * 2), rect.height() - (borderSize * 2)};
    _canvas.draw_nine_patch(np.Texture.ptr(), np.Region, rect, center, np.UV);
}

void widget_painter::draw_border(rect_f const& rect, element::border const& borderStyle, f32 borderSize, f32 borderRadius)
{
    if (borderSize <= 0.0f) { return; }

    if (auto const* np {std::get_if<nine_patch>(&borderStyle.Background)}) {
        draw_nine_patch(*np, rect, borderStyle);
    } else {
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
            _canvas.rounded_rect({rect.left() - (dborderSize * 2), rect.top() - (dborderSize * 2), rect.width() + (dborderSize * 4), rect.height() + (dborderSize * 4)}, borderRadius);
            _canvas.rounded_rect(rect, borderRadius);
            _canvas.stroke();

        } break;
        case element::border::type::Dashed: {
            _canvas.set_stroke_style(get_paint(borderStyle.Background, rect));
            _canvas.set_stroke_width(borderSize);

            std::vector<f32> dash;
            dash.reserve(borderStyle.Dash.size());
            for (auto const& l : borderStyle.Dash) {
                dash.push_back(l.calc(rect.width()));
            }
            _canvas.set_line_dash(dash);
            _canvas.set_dash_offset(borderStyle.DashOffset);
            _canvas.begin_path();
            _canvas.rounded_rect(rect, borderRadius);
            _canvas.stroke();
            _canvas.set_line_dash({});
        } break;
        case element::border::type::Dotted: {
            _canvas.set_stroke_style(get_paint(borderStyle.Background, rect));
            _canvas.set_stroke_width(borderSize);

            _canvas.set_line_dash(std::array {borderSize / 2, borderSize * 2});
            _canvas.set_line_cap(line_cap::Round);
            _canvas.set_dash_offset(borderStyle.DashOffset);
            _canvas.begin_path();
            _canvas.rounded_rect(rect, borderRadius);
            _canvas.stroke();
            _canvas.set_line_dash({});
            _canvas.set_line_cap(line_cap::Butt);
        } break;
        case element::border::type::Hidden:
            break;
        }
    }
}

void widget_painter::draw_text(element::text const& style, rect_f const& rect, utf8_string const& text)
{
    if (text.empty()) { return; }

    draw_text(style, rect, format_text(style, rect, text));
}

void widget_painter::draw_text(element::text const& style, rect_f const& rect, text_formatter::result const& text)
{
    auto const guard {_canvas.create_guard()};

    _canvas.set_font(text.Font);
    _canvas.set_text_halign(style.Alignment.Horizontal);
    _canvas.set_text_valign(style.Alignment.Vertical);

    // shadow
    if (style.Shadow.Color.A != 0) {
        point_f const dropShadowOffset {style.Shadow.OffsetX.calc(rect.width()),
                                        style.Shadow.OffsetY.calc(rect.height())};

        _canvas.set_fill_style(style.Shadow.Color);
        rect_f shadowRect {rect};
        shadowRect.Position.X += dropShadowOffset.X;
        shadowRect.Position.Y += dropShadowOffset.Y;
        _canvas.draw_text(shadowRect.Position, text);
    }

    // text
    _canvas.set_fill_style(style.Color);
    _canvas.draw_text(rect.Position, text);

    // deco
    auto const& deco {style.Decoration};
    if ((deco.Line.LineThrough || deco.Line.Overline || deco.Line.Underline) && deco.Color.A > 0) {
        f32 const strokeWidth {deco.Size.calc(rect.height())};

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
                        _canvas.move_to(p0 + offset);
                        _canvas.set_line_dash(std::array {strokeWidth / 4, strokeWidth * 2});
                        _canvas.set_line_cap(line_cap::Round);
                        _canvas.line_to(p1 + offset);
                        _canvas.fill();
                        _canvas.set_line_dash({});
                        break;
                    case text_decoration::style::Dashed:
                        _canvas.set_stroke_width(strokeWidth);
                        _canvas.set_stroke_style(deco.Color);
                        _canvas.begin_path();
                        _canvas.move_to(p0 + offset);
                        _canvas.set_line_dash(std::vector {0.025f, 0.025f});
                        _canvas.line_to(p1 + offset);
                        _canvas.stroke();
                        _canvas.set_line_dash({});
                        break;
                    case text_decoration::style::Wavy:
                        _canvas.set_stroke_width(strokeWidth);
                        _canvas.set_stroke_style(deco.Color);
                        _canvas.begin_path();
                        _canvas.move_to(p0 + offset);
                        _canvas.wavy_line_to(p1 + offset, strokeWidth * 1.25f, 0.5f);
                        _canvas.stroke();
                        break;
                    }
                }};

                point_f offset {rect.top_left()};

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
                    offset += point_f {0, first.height() / 3 * 2};
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

void widget_painter::draw_text_and_icon(element::text const& style, rect_f const& rect, utf8_string const& text, icon const& icon)
{
    rect_f contentRect {rect};

    bool const drawText {!text.empty() && style.Font};
    bool const drawIcon {icon.Texture};

    if (drawText && drawIcon) {
        rect_f textRect {contentRect};
        textRect.Size.Width /= 2;
        textRect.Position.X += textRect.width();

        draw_text(style, textRect, text);

        rect_f iconRect {contentRect};
        iconRect.Size.Width /= 2;

        size_f const iconSize {iconRect.Size.as_fitted(size_f {icon.Texture->info().Size})};
        iconRect = {iconRect.Position, iconSize};
        iconRect.Position.Y += (contentRect.height() - iconRect.height()) / 2;

        _canvas.set_fill_style(style.Color);
        _canvas.draw_image(icon.Texture.ptr(), icon.Region, iconRect);
    } else if (drawText) {
        draw_text(style, contentRect, text);
    } else if (drawIcon) {
        size_f const iconSize {contentRect.Size.as_fitted(size_f {icon.Texture->info().Size})};
        contentRect = {{contentRect.center().X - (iconSize.Width / 2), contentRect.top()}, iconSize};

        _canvas.set_fill_style(style.Color);
        _canvas.draw_image(icon.Texture.ptr(), icon.Region, contentRect);
    }
}

void widget_painter::draw_tick(element::tick const& style, rect_f const& rect)
{
    auto const guard {_canvas.create_guard()};

    if (auto const* np {std::get_if<nine_patch>(&style.Foreground)}) {
        draw_nine_patch(*np, rect, {});
    } else {
        switch (style.Type) {
        case element::tick::type::Checkmark: {
            f32 const width {style.Size.calc(std::min(rect.height(), rect.width())) / 4};
            _canvas.set_stroke_width(width);
            _canvas.set_stroke_style(get_paint(style.Foreground, rect));
            _canvas.begin_path();
            _canvas.move_to({rect.left(), rect.center().Y});
            _canvas.line_to({rect.center().X, rect.bottom()});
            _canvas.line_to(rect.top_right());
            _canvas.stroke();
        } break;
        case element::tick::type::Cross: {
            f32 const width {style.Size.calc(std::min(rect.height(), rect.width())) / 4};
            _canvas.set_stroke_width(width);
            _canvas.set_stroke_style(get_paint(style.Foreground, rect));
            _canvas.begin_path();
            _canvas.move_to(rect.top_left());
            _canvas.line_to(rect.bottom_right());
            _canvas.move_to(rect.top_right());
            _canvas.line_to(rect.bottom_left());
            _canvas.stroke();
        } break;
        case element::tick::type::Circle: {
            f32 const width {style.Size.calc(std::min(rect.height(), rect.width())) / 3};
            _canvas.set_stroke_width(width);
            _canvas.set_stroke_style(get_paint(style.Foreground, rect));
            _canvas.begin_path();
            _canvas.circle(rect.center(), width);
            _canvas.stroke();
        } break;
        case element::tick::type::Disc: {
            f32 const width {style.Size.calc(std::min(rect.height(), rect.width())) / 2};
            _canvas.set_fill_style(get_paint(style.Foreground, rect));
            _canvas.begin_path();
            _canvas.circle(rect.center(), width);
            _canvas.fill();
        } break;
        case element::tick::type::Rect: {
            rect_f const newRect {rect.left() + style.Size.calc(rect.width()),
                                  rect.top() + style.Size.calc(rect.height()),
                                  rect.width() - (style.Size.calc(rect.width()) + style.Size.calc(rect.width())),
                                  rect.height() - (style.Size.calc(rect.height()) + style.Size.calc(rect.height()))};
            _canvas.set_fill_style(get_paint(style.Foreground, newRect));
            _canvas.begin_path();
            _canvas.rect(newRect);
            _canvas.fill();
        } break;
        case element::tick::type::Square: {
            f32 const    width {style.Size.calc(std::min(rect.height(), rect.width()))};
            rect_f const newRect {rect.center() - point_f {width, width} / 2, {width, width}};
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

auto widget_painter::draw_bar(element::bar const& style, rect_f const& rect, element::bar::context const& barCtx) -> rect_f
{
    rect_f retValue {style.calc(rect, barCtx.Orientation, barCtx.Position)};
    rect_f lowRect {retValue};

    if (style.Type == element::bar::type::Continuous) {
        // Higher
        draw_bordered_rect(retValue, style.HigherBackground, style.Border);
        if (style.HigherBackground != style.LowerBackground) {
            // Lower
            switch (barCtx.Orientation) {
            case orientation::Horizontal:
                lowRect.Size.Width *= (barCtx.Inverted ? 1.0f - barCtx.Fraction : barCtx.Fraction);
                break;
            case orientation::Vertical:
                lowRect.Size.Height *= (barCtx.Inverted ? 1.0f - barCtx.Fraction : barCtx.Fraction);
                lowRect.Position.Y += retValue.height() - lowRect.height();
                break;
            }
            if (lowRect.height() > 0 && lowRect.width() > 0) {
                draw_bordered_rect(lowRect, style.LowerBackground, style.Border);
            }
        }
    } else if (style.Type == element::bar::type::Blocks) {
        for (i32 i {0}; i < barCtx.BlockCount; ++i) {
            rect_f   blockRect {retValue};
            ui_paint back;

            switch (barCtx.Orientation) {
            case orientation::Horizontal:
                blockRect.Size.Width /= barCtx.BlockCount;
                blockRect.Position.X += blockRect.width() * i;
                back = i / static_cast<f32>(barCtx.BlockCount) < (barCtx.Inverted ? 1.0f - barCtx.Fraction : barCtx.Fraction)
                    ? style.LowerBackground
                    : style.HigherBackground;
                break;
            case orientation::Vertical:
                blockRect.Size.Height /= barCtx.BlockCount;
                blockRect.Position.Y += blockRect.height() * i;
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

auto widget_painter::draw_thumb(element::thumb const& style, rect_f const& rect, element::thumb::context const& thumbCtx) -> rect_f
{
    rect_f retValue {style.calc(rect, thumbCtx)};

    if (style.Type == element::thumb::type::Rect) {
        draw_bordered_rect(retValue, style.Background, style.Border);
    } else if (style.Type == element::thumb::type::Disc) {
        draw_bordered_circle(retValue, style.Background, style.Border);
    }

    return retValue;
}

void widget_painter::draw_nav_arrow(element::nav_arrow const& style, rect_f const& rect)
{
    auto const guard {_canvas.create_guard()};

    rect_f const navRect {style.calc(rect)};

    rect_f decRect {navRect};
    draw_bordered_rect(decRect, style.DownBackground, style.Border);

    switch (style.Type) {
    case element::nav_arrow::type::Triangle: {
        _canvas.set_fill_style(get_paint(style.Foreground, navRect));
        _canvas.begin_path();
        _canvas.triangle(
            {navRect.left() + 2, navRect.top() + 4},
            {navRect.center().X, navRect.bottom() - 4},
            {navRect.right() - 2, navRect.top() + 4});
        _canvas.fill();
    } break;
    case element::nav_arrow::type::None:
        break;
    }
}

auto widget_painter::draw_nav_arrows(element::nav_arrow const& incStyle, element::nav_arrow const& decStyle, rect_f const& rect) -> std::pair<rect_f, rect_f>
{
    std::pair<rect_f, rect_f> retValue;

    auto const guard {_canvas.create_guard()};

    {
        rect_f const navRect {decStyle.calc(rect)};

        rect_f decRect {navRect};
        decRect.Size.Height /= 2;
        decRect.Position.Y += decRect.height();
        draw_bordered_rect(decRect, decStyle.DownBackground, decStyle.Border);

        if (auto const* np {std::get_if<nine_patch>(&decStyle.Foreground)}) {
            draw_nine_patch(*np, decRect, decStyle.Border);
        } else {
            switch (decStyle.Type) {
            case element::nav_arrow::type::Triangle: {
                point_f const center {navRect.center()};
                _canvas.set_fill_style(get_paint(decStyle.Foreground, decRect));
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

            retValue.second = decRect;
        }
    }

    {
        rect_f const navRect {incStyle.calc(rect)};

        rect_f incRect {navRect};
        incRect.Size.Height /= 2;
        draw_bordered_rect(incRect, incStyle.UpBackground, incStyle.Border);

        if (auto const* np {std::get_if<nine_patch>(&incStyle.Foreground)}) {
            draw_nine_patch(*np, incRect, incStyle.Border);
        } else {
            switch (incStyle.Type) {
            case element::nav_arrow::type::Triangle: {
                point_f const center {navRect.center()};
                _canvas.set_fill_style(get_paint(incStyle.Foreground, incRect));
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

        retValue.first = incRect;
    }

    return retValue;
}

void widget_painter::draw_item(element::item const& style, rect_f const& rect, list_item const& item)
{
    draw_bordered_rect(rect, style.Background, style.Border);

    rect_f contentRect {rect};
    contentRect -= style.Padding;
    contentRect -= style.Border.thickness();
    draw_text_and_icon(style.Text, contentRect, item.Text, item.Icon);
}

void widget_painter::draw_caret(element::caret const& style, rect_f const& rect, point_f offset)
{
    rect_f r {rect};
    r.Size.Width = style.Width.calc(rect.Size.Width);
    r.Position += offset;
    draw_bordered_rect(r, style.Color, {});
}

void widget_painter::draw_shadow(element::shadow const& style, rect_f const& rect, bool isCircle, element::border const& borderStyle)
{
    if (style.Color.A != 0) {
        point_f const dropShadowOffset {style.OffsetX.calc(rect.width()),
                                        style.OffsetY.calc(rect.height())};

        rect_f shadowRect {rect};
        shadowRect.Position.X += dropShadowOffset.X;
        shadowRect.Position.Y += dropShadowOffset.Y;

        if (isCircle) {
            draw_bordered_circle(shadowRect, style.Color, {.Radius = borderStyle.Radius});
        } else {
            draw_bordered_rect(shadowRect, style.Color, {.Radius = borderStyle.Radius});
        }
    }
}

auto widget_painter::canvas() -> gfx::canvas&
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

    auto* const font {style.Font->get_font(style.Style, fontSize).ptr()};

    _canvas.set_font(font);
    _canvas.set_text_halign(style.Alignment.Horizontal);
    _canvas.set_text_valign(style.Alignment.Vertical);

    auto const rectSize {rect.Size};

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

auto widget_painter::get_paint(ui_paint const& p, rect_f const& rect) -> canvas::paint
{
    return std::visit(
        overloaded {
            [&](color const& arg) -> canvas::paint {
                return canvas::paint {
                    .Feather = 1.0f,
                    .Color   = arg,
                };
            },
            [&](linear_gradient const& arg) -> canvas::paint {
                degree_f const angle {arg.Angle + degree_f {90}};
                return _canvas.create_linear_gradient(
                    rect.find_edge(angle),
                    rect.find_edge(angle - degree_f {180}),
                    arg.Colors);
            },
            [&](radial_gradient const& arg) -> canvas::paint {
                return _canvas.create_radial_gradient(
                    rect.center(),
                    arg.InnerRadius.calc(rect.width()), arg.OuterRadius.calc(rect.width()),
                    arg.Scale, arg.Colors);
            },
            [&](box_gradient const& arg) -> canvas::paint {
                return _canvas.create_box_gradient(
                    rect,
                    arg.Radius.calc(rect.width()), arg.Feather.calc(rect.width()), arg.Colors);
            },
            [&](nine_patch const&) -> canvas::paint {
                return {};
            },
        },
        p);
}

////////////////////////////////////////////////////////////

scissor_guard::scissor_guard(widget_painter& painter, widget* w)
    : _painter {painter}
{
    rect_f bounds {w->global_content_bounds()};

    if (auto const* form {w->parent_form()}) {
        point_f const off {form->Bounds->Position};
        bounds.Position.X -= off.X;
        bounds.Position.Y -= off.Y;
    }
    painter.push_scissor(bounds);
}

scissor_guard::~scissor_guard()
{
    _painter.pop_scissor();
}

} // namespace ui
