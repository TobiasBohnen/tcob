// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/html/HtmlElementPainter.hpp"

#include "tcob/core/Point.hpp"

#if defined(TCOB_ENABLE_ADDON_GFX_LITEHTML)

namespace tcob::gfx::html {

element_painter::element_painter(canvas& c)
    : _canvas {c}
{
}

auto element_painter::get_canvas() -> canvas&
{
    return _canvas;
}

void element_painter::draw_background(background_draw_context const& ctx)
{
    _canvas.save();

    _canvas.set_scissor(ctx.ClipBox);

    if (ctx.Gradient) {
        degree_f const angle {ctx.Gradient->Angle + degree_f {90}};
        point_f const  p0 {ctx.OriginBox.find_edge(angle)};
        point_f const  p1 {ctx.OriginBox.find_edge(angle - degree_f {180})};
        _canvas.set_fill_style(_canvas.create_linear_gradient(p0, p1, ctx.Gradient->Colors));
    } else {
        _canvas.set_fill_style(ctx.BackgroundColor);
    }

    _canvas.fill_rounded_rect_varying(ctx.ClipBox, ctx.BorderRadii.TopLeft, ctx.BorderRadii.TopRight, ctx.BorderRadii.BottomRight, ctx.BorderRadii.BottomLeft);

    if (ctx.Image) { // image optional?
        if (ctx.Repeat == background_repeat::NoRepeat) {
            _canvas.draw_image(ctx.Image, ctx.OriginBox);
        } else {
            auto const imgPattern {_canvas.create_image_pattern(ctx.OriginBox.top_left(), ctx.ImageSize, degree_f {0.0f}, ctx.Image, 1)};
            _canvas.set_fill_style(imgPattern);

            switch (ctx.Repeat) {
            case background_repeat::Repeat:
                _canvas.fill_rect(ctx.OriginBox);
                break;
            case background_repeat::RepeatX:
                _canvas.fill_rect({ctx.OriginBox.top_left(), {ctx.OriginBox.Width, ctx.ImageSize.Height}});
                break;
            case background_repeat::RepeatY:
                _canvas.fill_rect({ctx.OriginBox.top_left(), {ctx.ImageSize.Width, ctx.OriginBox.Height}});
                break;
            case background_repeat::NoRepeat:
                break;
            }
        }
    }

    _canvas.restore();
}

void element_painter::draw_borders(borders const& brds)
{
    _canvas.save();
    if (brds.Left.Width > 0) {
        draw_left_border(brds);
    }
    if (brds.Top.Width > 0) {
        draw_top_border(brds);
    }
    if (brds.Right.Width > 0) {
        draw_right_border(brds);
    }
    if (brds.Bottom.Width > 0) {
        draw_bottom_border(brds);
    }
    _canvas.restore();
}

void element_painter::draw_text(text_draw_context const& ctx)
{
    using namespace tcob::enum_ops;

    _canvas.save();

    _canvas.set_fill_style(ctx.TextColor);
    _canvas.set_font(ctx.Font);
    _canvas.draw_textbox(ctx.TextBox, ctx.Text);

    if (ctx.FontDecorations != font_decorations::None) {
        _canvas.set_stroke_style(ctx.TextColor);
        _canvas.set_stroke_width(std::max(ctx.TextBox.Height / 10, 3.0f));

        if ((ctx.FontDecorations & font_decorations::Linethrough) == font_decorations::Linethrough) {
            _canvas.stroke_line({ctx.TextBox.left(), ctx.TextBox.top() + ctx.TextBox.Height / 2}, {ctx.TextBox.right(), ctx.TextBox.top() + ctx.TextBox.Height / 2});
        }
        if ((ctx.FontDecorations & font_decorations::Overline) == font_decorations::Overline) {
            _canvas.stroke_line({ctx.TextBox.left(), ctx.TextBox.top()}, {ctx.TextBox.right(), ctx.TextBox.top()});
        }
        if ((ctx.FontDecorations & font_decorations::Underline) == font_decorations::Underline) {
            _canvas.stroke_line({ctx.TextBox.left(), ctx.TextBox.bottom()}, {ctx.TextBox.right(), ctx.TextBox.bottom()});
        }
    }

    _canvas.restore();
}

void element_painter::draw_left_border(borders const& brds)
{
    auto const& border {brds.Left};
    if (border.Style == border_style::Solid) {
        _canvas.set_stroke_style(border.Color);
        _canvas.set_stroke_width(border.Width);
        _canvas.set_line_cap(line_cap::Butt);

        _canvas.begin_path();

        _canvas.move_to({brds.DrawBox.left() + brds.BorderRadii.BottomLeft, brds.DrawBox.bottom()});
        _canvas.quad_bezier_to({brds.DrawBox.left(), brds.DrawBox.bottom()}, {brds.DrawBox.left(), brds.DrawBox.bottom() - brds.BorderRadii.BottomLeft});

        _canvas.line_to({brds.DrawBox.left(), brds.DrawBox.top() + brds.BorderRadii.TopLeft});
        _canvas.quad_bezier_to({brds.DrawBox.left(), brds.DrawBox.top()}, {brds.DrawBox.left() + brds.BorderRadii.TopLeft, brds.DrawBox.top()});

        _canvas.stroke();
    }
}

void element_painter::draw_top_border(borders const& brds)
{
    auto const& border {brds.Top};
    if (border.Style == border_style::Solid) {
        _canvas.set_stroke_style(border.Color);
        _canvas.set_stroke_width(border.Width);
        _canvas.set_line_cap(line_cap::Square);

        _canvas.begin_path();

        _canvas.move_to({brds.DrawBox.left() + brds.BorderRadii.TopLeft, brds.DrawBox.top()});
        _canvas.line_to({brds.DrawBox.right() - brds.BorderRadii.TopRight, brds.DrawBox.top()});

        _canvas.stroke();
    }
}

void element_painter::draw_right_border(borders const& brds)
{
    auto const& border {brds.Right};
    if (border.Style == border_style::Solid) {
        _canvas.set_stroke_style(border.Color);
        _canvas.set_stroke_width(border.Width);
        _canvas.set_line_cap(line_cap::Butt);

        _canvas.begin_path();

        _canvas.move_to({brds.DrawBox.right() - brds.BorderRadii.TopRight, brds.DrawBox.top()});
        _canvas.quad_bezier_to({brds.DrawBox.right(), brds.DrawBox.top()}, {brds.DrawBox.right(), brds.DrawBox.top() + brds.BorderRadii.TopRight});

        _canvas.line_to({brds.DrawBox.right(), brds.DrawBox.bottom() - brds.BorderRadii.BottomRight});
        _canvas.quad_bezier_to({brds.DrawBox.right(), brds.DrawBox.bottom()}, {brds.DrawBox.right() - brds.BorderRadii.BottomRight, brds.DrawBox.bottom()});

        _canvas.stroke();
    }
}

void element_painter::draw_bottom_border(borders const& brds)
{
    auto const& border {brds.Bottom};
    if (border.Style == border_style::Solid) {
        _canvas.set_stroke_style(border.Color);
        _canvas.set_stroke_width(border.Width);
        _canvas.set_line_cap(line_cap::Square);

        _canvas.begin_path();

        _canvas.move_to({brds.DrawBox.right() - brds.BorderRadii.BottomRight, brds.DrawBox.bottom()});
        _canvas.line_to({brds.DrawBox.left() + brds.BorderRadii.BottomLeft, brds.DrawBox.bottom()});

        _canvas.stroke();
    }
}

void element_painter::draw_list_marker(list_marker_draw_context const& ctx)
{
    _canvas.save();

    switch (ctx.Type) {
    case list_marker_type::Circle:
        _canvas.set_stroke_style(ctx.Color);
        _canvas.stroke_circle(ctx.Box.get_center(), ctx.Box.Width);
        break;
    case list_marker_type::Disc:
        _canvas.set_fill_style(ctx.Color);
        _canvas.fill_circle(ctx.Box.get_center(), ctx.Box.Width);
        break;
    case list_marker_type::Square:
        _canvas.set_fill_style(ctx.Color);
        _canvas.fill_rect(ctx.Box);
        break;
    case list_marker_type::Image:
        _canvas.draw_image(ctx.Image, ctx.Box);
        break;
    }

    _canvas.restore();
}

////////////////////////////////////////////////////////////

auto margins::width() const -> i32
{
    return Left + Right;
}

auto margins::height() const -> i32
{
    return Top + Bottom;
}

}

#endif
