// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/WidgetPainter.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>
#include <variant>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/TextFormatter.hpp"
#include "tcob/gfx/Transform.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

widget_painter::widget_painter(gfx::canvas& canvas)
    : _canvas {canvas}
{
}

void widget_painter::begin(f32 alpha)
{
    _canvas.save();
    _canvas.set_global_alpha(alpha);
}

void widget_painter::begin(f32 alpha, gfx::transform const& xform)
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

void widget_painter::add_overlay(overlay_func const& func)
{
    _overlays.push_back(func);
}

void widget_painter::draw_overlays()
{
    if (_overlays.empty()) { return; }

    _canvas.reset();

    for (auto& func : _overlays) {
        func(*this);
    }
    _overlays.clear();

    _canvas.reset();
}

////////////////////////////////////////////////////////////

auto static fill(gfx::canvas& canvas, auto&& paint, auto&& func)
{
    canvas.set_fill_style(paint);
    canvas.begin_path();
    func();
    canvas.fill();
}

auto static stroke(gfx::canvas& canvas, f32 size, auto&& paint, auto&& func)
{
    canvas.set_stroke_width(size);
    canvas.set_stroke_style(paint);
    canvas.begin_path();
    func();
    canvas.stroke();
}

void widget_painter::draw_background_and_border(widget_style const& style, rect_f& rect, bool isCircle)
{
    //  add margin
    rect -= style.Margin;

    draw_shadow(style.DropShadow, rect, isCircle, style.Border);
    if (isCircle) {
        draw_bordered_circle(rect, style.Background, style.Border);
    } else {
        draw_bordered_rect(rect, style.Background, style.Border);
    }

    // add padding
    rect -= style.Padding;

    // add border
    rect -= style.Border.thickness();
}

void widget_painter::draw_bordered_rect(rect_f const& rect, ui_paint const& back, border_element const& borderStyle)
{
    auto const guard {_canvas.create_guard()};

    if (auto const* np {std::get_if<nine_patch>(&back)}) {
        draw_nine_patch(*np, rect, borderStyle);
    } else {
        // background
        f32 const borderRadius {borderStyle.Radius.calc(rect.width())};
        fill(_canvas, get_paint(back, rect), [&] {
            _canvas.rounded_rect(rect, borderRadius);
        });

        // border
        f32 const borderSize {borderStyle.Size.calc(rect.width())};
        draw_border(rect, borderStyle, borderSize, borderRadius);
    }
}

void widget_painter::draw_bordered_circle(rect_f const& rect, ui_paint const& back, border_element const& borderStyle)
{
    auto const guard {_canvas.create_guard()};

    if (auto const* np {std::get_if<nine_patch>(&back)}) {
        draw_nine_patch(*np, rect, borderStyle);
    } else {
        // background
        f32 const r {std::min(rect.height(), rect.width()) / 2};
        fill(_canvas, get_paint(back, rect), [&] {
            _canvas.circle(rect.center(), r);
        });

        // border
        f32 const borderSize {borderStyle.Size.calc(rect.width())};
        draw_border({rect.left() + (rect.width() / 2 - r), rect.top() + (rect.height() / 2 - r), r * 2, r * 2}, borderStyle, borderSize, r);
    }
}

void widget_painter::draw_nine_patch(nine_patch const& np, rect_f const& rect, border_element const& borderStyle)
{
    _canvas.set_fill_style(colors::White);
    f32 const    borderSize {borderStyle.Size.calc(rect.width())};
    rect_f const center {rect.left() + borderSize, rect.top() + borderSize, rect.width() - (borderSize * 2), rect.height() - (borderSize * 2)};
    _canvas.draw_nine_patch(np.Texture.ptr(), np.TextureRegion, rect, center, np.UV);
}

void widget_painter::draw_border(rect_f const& rect, border_element const& borderStyle, f32 borderSize, f32 borderRadius)
{
    if (borderSize <= 0.0f) { return; }

    if (auto const* np {std::get_if<nine_patch>(&borderStyle.Background)}) {
        draw_nine_patch(*np, rect, borderStyle);
    } else {
        switch (borderStyle.Type) {
        case border_type::Solid: {
            stroke(_canvas, borderSize, get_paint(borderStyle.Background, rect), [&] {
                _canvas.rounded_rect(rect, borderRadius);
            });
        } break;
        case border_type::Double: {
            f32 const dborderSize {borderSize / 3};
            stroke(_canvas, dborderSize, get_paint(borderStyle.Background, rect), [&] {
                _canvas.rounded_rect({rect.left() - (dborderSize * 2), rect.top() - (dborderSize * 2), rect.width() + (dborderSize * 4), rect.height() + (dborderSize * 4)}, borderRadius);
                _canvas.rounded_rect(rect, borderRadius);
            });
        } break;
        case border_type::Dashed: {
            std::vector<f32> dash;
            dash.reserve(borderStyle.Dash.size());
            for (auto const& l : borderStyle.Dash) { dash.push_back(l.calc(rect.width())); }
            _canvas.set_line_dash(dash);
            _canvas.set_dash_offset(borderStyle.DashOffset);
            stroke(_canvas, borderSize, get_paint(borderStyle.Background, rect), [&] {
                _canvas.rounded_rect(rect, borderRadius);
            });
            _canvas.set_line_dash({});
        } break;
        case border_type::Dotted: {

            _canvas.set_line_dash(std::array {borderSize / 2, borderSize * 2});
            _canvas.set_line_cap(gfx::line_cap::Round);
            _canvas.set_dash_offset(borderStyle.DashOffset);
            stroke(_canvas, borderSize, get_paint(borderStyle.Background, rect), [&] {
                _canvas.rounded_rect(rect, borderRadius);
            });
            _canvas.set_line_dash({});
            _canvas.set_line_cap(gfx::line_cap::Butt);
        } break;
        case border_type::Cornered: {
            auto const drawCorner {[&](point_f p0, point_f p1, point_f p2) {
                _canvas.move_to(p0);
                _canvas.arc_to(p1, p2, borderRadius);
                _canvas.line_to(p2);
            }};

            f32 const len {std::min(rect.width(), rect.height()) * 0.25f};
            stroke(_canvas, borderSize, get_paint(borderStyle.Background, rect), [&] {
                drawCorner({rect.left() + len, rect.top()}, rect.top_left(), {rect.left(), rect.top() + len});
                drawCorner({rect.right() - len, rect.top()}, rect.top_right(), {rect.right(), rect.top() + len});
                drawCorner({rect.right(), rect.bottom() - len}, rect.bottom_right(), {rect.right() - len, rect.bottom()});
                drawCorner({rect.left(), rect.bottom() - len}, rect.bottom_left(), {rect.left() + len, rect.bottom()});
            });
        } break;
        case border_type::Centered: {
            auto const drawEdge {[&](point_f from, point_f to) {
                _canvas.move_to(from);
                _canvas.line_to(to);
            }};

            f32 const len {std::min(rect.width(), rect.height()) * 0.25f};
            stroke(_canvas, borderSize, get_paint(borderStyle.Background, rect), [&] {
                drawEdge({rect.left() + len, rect.top()}, {rect.right() - len, rect.top()});
                drawEdge({rect.right(), rect.top() + len}, {rect.right(), rect.bottom() - len});
                drawEdge({rect.right() - len, rect.bottom()}, {rect.left() + len, rect.bottom()});
                drawEdge({rect.left(), rect.bottom() - len}, {rect.left(), rect.top() + len});
            });
        } break;
        case border_type::Wavy: {
            stroke(_canvas, borderSize, get_paint(borderStyle.Background, rect), [&] {
                _canvas.move_to(rect.top_left());
                _canvas.wavy_line_to(rect.top_right(), borderSize / 3, 1);
                _canvas.wavy_line_to(rect.bottom_right(), borderSize / 3, 1);
                _canvas.wavy_line_to(rect.bottom_left(), borderSize / 3, 1);
                _canvas.wavy_line_to(rect.top_left(), borderSize / 3, 1);
            });
        } break;
        case border_type::Inset:
        case border_type::Outset: {
            auto const base {get_paint(borderStyle.Background, rect)};
            if (std::get_if<gfx::paint_gradient>(&base.Color)) {
                stroke(_canvas, borderSize, get_paint(borderStyle.Background, rect), [&] {
                    _canvas.move_to(rect.top_left());
                    _canvas.rounded_rect(rect, borderRadius);
                });
            } else if (auto const* col {std::get_if<color>(&base.Color)}) {
                hsx const h {col->to_hsl()};
                hsx       bright {h};
                hsx       dark {h};
                if (h.X < 0.25f) {
                    bright.X = h.X + 0.25f;
                } else {
                    dark.X = h.X - 0.25f;
                }
                color const brightCol {color::FromHSLA(bright, col->A)};
                color const darkCol {color::FromHSLA(dark, col->A)};

                rect_f const clipRect {rect.as_padded_by({-borderSize, -borderSize})};

                _canvas.begin_path();
                _canvas.triangle(clipRect.bottom_left(), clipRect.top_right(), clipRect.bottom_right());
                _canvas.clip();
                stroke(_canvas, borderSize, borderStyle.Type == border_type::Inset ? brightCol : darkCol, [&] {
                    _canvas.rounded_rect(rect, borderRadius);
                });

                _canvas.begin_path();
                _canvas.triangle(clipRect.top_left(), clipRect.top_right(), clipRect.bottom_left());
                _canvas.clip();
                stroke(_canvas, borderSize, borderStyle.Type == border_type::Inset ? darkCol : brightCol, [&] {
                    _canvas.rounded_rect(rect, borderRadius);
                });

                _canvas.reset_clip();
            }
        } break;

        case border_type::Hidden: break;
        }
    }
}

void widget_painter::draw_text(text_element const& style, rect_f const& rect, utf8_string const& text)
{
    if (text.empty()) { return; }

    draw_text(style, rect, format_text(style, rect, text));
}

void widget_painter::draw_text(text_element const& style, rect_f const& rect, gfx::text_formatter::result const& text)
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
    if (deco.Style != line_type::Hidden && (deco.Line.LineThrough || deco.Line.Overline || deco.Line.Underline) && deco.Color.A > 0 && text.QuadCount > 0) {
        f32 const strokeWidth {deco.Size.calc(rect.height())};

        // FIXME: multiline
        rect_f first {text.get_quad(0).Rect};
        rect_f last {text.get_quad(text.QuadCount - 1).Rect};

        auto const drawDeco {[&]() {
            auto const drawLine {[&](point_f p0, point_f p1, point_f offset) {
                switch (deco.Style) {
                case line_type::Solid:
                case line_type::Double: // drawn twice
                    stroke(_canvas, strokeWidth, deco.Color, [&] {
                        _canvas.move_to(p0 + offset);
                        _canvas.line_to(p1 + offset);
                    });
                    break;
                case line_type::Dotted: {
                    f32 const dash {std::max(1.0f, static_cast<f32>(p0.distance_to(p1) / 20))};
                    _canvas.set_line_dash(std::array {dash, dash * 2});
                    stroke(_canvas, strokeWidth, deco.Color, [&] {
                        _canvas.move_to(p0 + offset);
                        _canvas.line_to(p1 + offset);
                    });
                    _canvas.set_line_dash({});
                } break;
                case line_type::Dashed: {
                    f32 const dash {std::max(1.0f, static_cast<f32>(p0.distance_to(p1) / 7))};
                    _canvas.set_line_dash(std::array {dash, dash});
                    _canvas.set_dash_offset(dash / 2);
                    stroke(_canvas, strokeWidth, deco.Color, [&] {
                        _canvas.move_to(p0 + offset);
                        _canvas.line_to(p1 + offset);
                    });
                    _canvas.set_line_dash({});
                    _canvas.set_dash_offset(0);
                } break;
                case line_type::Wavy:
                    stroke(_canvas, strokeWidth, deco.Color, [&] {
                        _canvas.move_to(p0 + offset);
                        _canvas.wavy_line_to(p1 + offset, strokeWidth * 1.25f, 0.5f);
                    });
                    break;
                case line_type::Hidden:
                    break;
                }
            }};

            point_f offset {rect.top_left()};

            if (deco.Line.Underline) {
                point_f p0 {first.bottom_left()};
                point_f p1 {last.bottom_right()};
                p0.Y = p1.Y = std::max(p0.Y, p1.Y);
                offset += point_f {0, strokeWidth / 2};
                drawLine(p0, p1, offset);
                if (deco.Style == line_type::Double) {
                    offset += point_f {0, strokeWidth * 1.5f};
                    drawLine(p0, p1, offset);
                }
            }
            if (deco.Line.Overline) {
                point_f p0 {first.top_left()};
                point_f p1 {last.top_right()};
                p0.Y = p1.Y = std::min(p0.Y, p1.Y);
                offset -= point_f {0, strokeWidth / 2};
                drawLine(p0, p1, offset);
                if (deco.Style == line_type::Double) {
                    offset -= point_f {0, strokeWidth * 1.5f};
                    drawLine(p0, p1, offset);
                }
            }
            if (deco.Line.LineThrough) {
                point_f p0 {first.top_left()};
                point_f p1 {last.top_right()};
                p0.Y = p1.Y = (first.center().Y + last.center().Y) / 2;
                drawLine(p0, p1, offset);
                if (deco.Style == line_type::Double) {
                    offset -= point_f {0, strokeWidth * 1.5f};
                    drawLine(p0, p1, offset);
                }
            }
        }};

        drawDeco();
    }
}

void widget_painter::draw_text_and_icon(text_element const& style, rect_f const& rect, utf8_string const& text, icon const& icon)
{
    rect_f contentRect {rect};

    bool const drawText {!text.empty() && style.Font};
    bool const drawIcon {icon.Texture && icon.Texture->has_region(icon.TextureRegion)};

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

        _canvas.set_fill_style(icon.Color);
        _canvas.draw_image(icon.Texture.ptr(), icon.TextureRegion, iconRect);
    } else if (drawText) {
        draw_text(style, contentRect, text);
    } else if (drawIcon) {
        size_f const iconSize {contentRect.Size.as_fitted(size_f {icon.Texture->info().Size})};
        contentRect = {{contentRect.center().X - (iconSize.Width / 2), contentRect.top()}, iconSize};

        _canvas.set_fill_style(icon.Color);
        _canvas.draw_image(icon.Texture.ptr(), icon.TextureRegion, contentRect);
    }
}

void widget_painter::draw_tick(tick_element const& style, rect_f const& rect)
{
    auto const guard {_canvas.create_guard()};

    if (auto const* np {std::get_if<nine_patch>(&style.Foreground)}) {
        draw_nine_patch(*np, rect, {});
    } else {
        switch (style.Type) {
        case tick_type::Checkmark: {
            f32 const width {style.Size.calc(std::min(rect.height(), rect.width())) / 4};
            stroke(_canvas, width, get_paint(style.Foreground, rect), [&] {
                _canvas.move_to({rect.left(), rect.center().Y});
                _canvas.line_to({rect.center().X, rect.bottom()});
                _canvas.line_to(rect.top_right());
            });
        } break;
        case tick_type::Cross: {
            f32 const width {style.Size.calc(std::min(rect.height(), rect.width())) / 4};
            stroke(_canvas, width, get_paint(style.Foreground, rect), [&] {
                _canvas.move_to(rect.top_left());
                _canvas.line_to(rect.bottom_right());
                _canvas.move_to(rect.top_right());
                _canvas.line_to(rect.bottom_left());
            });
        } break;
        case tick_type::Circle: {
            f32 const width {style.Size.calc(std::min(rect.height(), rect.width())) / 3};
            stroke(_canvas, width, get_paint(style.Foreground, rect), [&] {
                _canvas.circle(rect.center(), width);
            });
        } break;
        case tick_type::Disc: {
            f32 const width {style.Size.calc(std::min(rect.height(), rect.width())) / 2};
            fill(_canvas, get_paint(style.Foreground, rect), [&] {
                _canvas.circle(rect.center(), width);
            });
        } break;
        case tick_type::Rect: {
            f32 const    width {style.Size.calc(rect.width())};
            f32 const    height {style.Size.calc(rect.height())};
            rect_f const newRect {rect.left() + ((rect.width() - width) / 2),
                                  rect.top() + ((rect.height() - height) / 2),
                                  width,
                                  height};
            fill(_canvas, get_paint(style.Foreground, newRect), [&] {
                _canvas.rect(newRect);
            });
        } break;
        case tick_type::Square: {
            f32 const    width {style.Size.calc(std::min(rect.height(), rect.width()))};
            rect_f const newRect {rect.center() - point_f {width, width} / 2, {width, width}};
            fill(_canvas, get_paint(style.Foreground, newRect), [&] {
                _canvas.rect(newRect);
            });
        } break;
        case tick_type::Triangle: {
            f32 const    width {style.Size.calc(rect.width())};
            f32 const    height {style.Size.calc(rect.height())};
            rect_f const newRect {rect.left() + ((rect.width() - width) / 2),
                                  rect.top() + ((rect.height() - height) / 2),
                                  width,
                                  height};
            fill(_canvas, get_paint(style.Foreground, newRect), [&] {
                _canvas.triangle(
                    {newRect.left() + 2, newRect.top() + 4},
                    {newRect.center().X, newRect.bottom() - 4},
                    {newRect.right() - 2, newRect.top() + 4});
            });
        } break;
        }
    }
}

auto widget_painter::draw_bar(bar_element const& style, rect_f const& rect, bar_element::context const& barCtx) -> rect_f
{
    rect_f retValue {style.calc(rect, barCtx.Orientation, barCtx.Position)};

    if (style.HigherBackground == style.LowerBackground || barCtx.Stops.size() < 3) {
        draw_bordered_rect(retValue, style.HigherBackground, style.Border);
        return retValue;
    }

    bool low {true};
    for (usize i {0}; i < barCtx.Stops.size() - 1; ++i) {
        f32 const start {barCtx.Stops[i]};
        f32 const end {barCtx.Stops[i + 1]};
        f32 const size {end - start};

        rect_f segRect {retValue};
        switch (barCtx.Orientation) {
        case orientation::Horizontal:
            segRect.Position.X += retValue.width() * start;
            segRect.Size.Width *= size;
            break;
        case orientation::Vertical:
            segRect.Position.Y += retValue.height() * (1 - end);
            segRect.Size.Height *= size;
            break;
        }
        if (segRect.height() > 0 && segRect.width() > 0) {
            draw_bordered_rect(segRect, low ? style.LowerBackground : style.HigherBackground, style.Border);
        }

        low = !low;
    }

    return retValue;
}

auto widget_painter::draw_thumb(thumb_element const& style, rect_f const& rect, thumb_element::context const& thumbCtx) -> rect_f
{
    rect_f retValue {style.calc(rect, thumbCtx)};

    switch (style.Type) {
    case thumb_type::Rect: draw_bordered_rect(retValue, style.Background, style.Border); break;
    case thumb_type::Disc: draw_bordered_circle(retValue, style.Background, style.Border); break;
    }

    return retValue;
}

void widget_painter::draw_chevron(nav_arrow_element const& style, rect_f const& rect, bool up)
{
    auto const guard {_canvas.create_guard()};

    rect_f const navRect {style.calc(rect)};

    rect_f decRect {navRect};
    draw_bordered_rect(decRect, up ? style.UpBackground : style.DownBackground, style.Border);

    if (auto const* np {std::get_if<nine_patch>(&style.Foreground)}) {
        draw_nine_patch(*np, decRect, style.Border);
    } else {
        if (up) {
            switch (style.Type) {
            case nav_arrow_type::Triangle: {
                fill(_canvas, get_paint(style.Foreground, navRect), [&] {
                    _canvas.triangle(
                        {navRect.left() + 2, navRect.bottom() - 4},
                        {navRect.center().X, navRect.top() + 4},
                        {navRect.right() - 2, navRect.bottom() - 4});
                });
            } break;
            }
        } else {
            switch (style.Type) {
            case nav_arrow_type::Triangle: {
                fill(_canvas, get_paint(style.Foreground, navRect), [&] {
                    _canvas.triangle(
                        {navRect.left() + 2, navRect.top() + 4},
                        {navRect.center().X, navRect.bottom() - 4},
                        {navRect.right() - 2, navRect.top() + 4});
                });
            } break;
            }
        }
    }
}

auto widget_painter::draw_nav_arrows(nav_arrow_element const& incStyle, nav_arrow_element const& decStyle, rect_f const& rect) -> std::pair<rect_f, rect_f>
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
            case nav_arrow_type::Triangle: {
                point_f const center {navRect.center()};
                fill(_canvas, get_paint(decStyle.Foreground, decRect), [&] {
                    _canvas.triangle(
                        {navRect.left() + 2, center.Y + 4},
                        {center.X, navRect.bottom() - 4},
                        {navRect.right() - 2, center.Y + 4});
                });
            } break;
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
            case nav_arrow_type::Triangle: {
                point_f const center {navRect.center()};
                fill(_canvas, get_paint(incStyle.Foreground, incRect), [&] {
                    _canvas.triangle(
                        {navRect.left() + 2, center.Y - 4},
                        {center.X, navRect.top() + 4},
                        {navRect.right() - 2, center.Y - 4});
                });
            } break;
            }
        }

        retValue.first = incRect;
    }

    return retValue;
}

void widget_painter::draw_item(item_element const& style, rect_f const& rect, item const& item)
{
    draw_bordered_rect(rect, style.Background, style.Border);

    rect_f contentRect {rect};
    contentRect -= style.Padding;
    contentRect -= style.Border.thickness();
    draw_text_and_icon(style.Text, contentRect, item.Text, item.Icon);
}

void widget_painter::draw_caret(caret_element const& style, rect_f const& rect, point_f offset)
{
    rect_f r {rect};
    r.Size.Width = style.Width.calc(rect.Size.Width);
    r.Position += offset;
    draw_bordered_rect(r, style.Color, {});
}

void widget_painter::draw_shadow(shadow_element const& style, rect_f const& rect, bool isCircle, border_element const& borderStyle)
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

auto widget_painter::format_text(text_element const& style, rect_f const& rect, utf8_string_view text) -> gfx::text_formatter::result
{
    auto const tt {transform_text(style.Transform, text)};
    return format_text(style, rect, tt, style.calc_font_size(rect), true);
}

////////////////////////////////////////////////////////////

auto widget_painter::format_text(text_element const& style, rect_f const& rect, utf8_string_view text, u32 fontSize, bool resize) -> gfx::text_formatter::result
{
    auto const guard {_canvas.create_guard()};

    auto* const font {style.Font->get_font(style.Style, fontSize).ptr()};

    _canvas.set_font(font);
    _canvas.set_text_halign(style.Alignment.Horizontal);
    _canvas.set_text_valign(style.Alignment.Vertical);

    auto const rectSize {rect.Size};

    f32 scale {1.0f};

    if (resize && style.AutoSize != auto_size_mode::Never) {
        auto const textSize {_canvas.measure_text(-1, text)};

        bool const shouldShrink {(style.AutoSize == auto_size_mode::Always || style.AutoSize == auto_size_mode::OnlyShrink)
                                 && (textSize.Width > rectSize.Width || textSize.Height > rectSize.Height)};

        bool const shouldGrow {(style.AutoSize == auto_size_mode::Always || style.AutoSize == auto_size_mode::OnlyGrow)
                               && (textSize.Width < rectSize.Width && textSize.Height < rectSize.Height)};

        if (shouldShrink || shouldGrow) {
            scale = std::min(std::floor(rectSize.Width) / textSize.Width, std::floor(rectSize.Height) / textSize.Height);
        }
    }

    return _canvas.format_text(rectSize, text, scale);
}

auto widget_painter::transform_text(text_transform xform, utf8_string_view text) const -> utf8_string
{
    utf8_string retValue {text};
    switch (xform) {
    case text_transform::Capitalize: retValue = utf8::capitalize(text); break;
    case text_transform::Lowercase:  retValue = utf8::to_lower(text); break;
    case text_transform::Uppercase:  retValue = utf8::to_upper(text); break;
    default:
        break;
    }
    return retValue;
}

auto widget_painter::get_paint(ui_paint const& p, rect_f const& rect) -> gfx::canvas::paint
{
    return std::visit(
        overloaded {
            [&](color const& arg) {
                return gfx::canvas::paint {
                    .Feather = 1.0f,
                    .Color   = arg,
                };
            },
            [&](linear_gradient const& arg) {
                degree_f const angle {arg.Angle + degree_f {90}};
                return _canvas.create_linear_gradient(
                    rect.find_edge(angle),
                    rect.find_edge(angle - degree_f {180}),
                    arg.Colors);
            },
            [&](radial_gradient const& arg) {
                return _canvas.create_radial_gradient(
                    rect.center(),
                    arg.InnerRadius.calc(rect.width()), arg.OuterRadius.calc(rect.width()),
                    arg.Scale, arg.Colors);
            },
            [&](box_gradient const& arg) {
                return _canvas.create_box_gradient(
                    rect,
                    arg.Radius.calc(rect.width()), arg.Feather.calc(rect.width()), arg.Colors);
            },
            [&](nine_patch const&) -> gfx::canvas::paint {
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
    bounds.Position -= w->form().Bounds->Position;
    painter.push_scissor(bounds);
}

scissor_guard::~scissor_guard()
{
    _painter.pop_scissor();
}

} // namespace ui
