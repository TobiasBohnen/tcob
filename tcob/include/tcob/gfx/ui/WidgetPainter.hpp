// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <stack>

#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API widget_painter {
public:
    explicit widget_painter(gfx::canvas& canvas);

    void begin(f32 alpha);
    void begin(f32 alpha, transform const& xform);
    void end();
    void push_scissor(rect_f const& globalScissor);
    void pop_scissor();

    void draw_background_and_border(background_style const& style, rect_f& refRect, bool isCircle);

    void draw_text(element::text const& style, rect_f const& refRect, utf8_string const& text);
    void draw_text(element::text const& style, rect_f const& refRect, text_formatter::result const& text);

    void draw_tick(element::tick const& style, rect_f const& refRect);
    void draw_nav_arrow(element::nav_arrow const& style, rect_f const& refRect);
    void draw_nav_arrows(element::nav_arrow const& incStyle, element::nav_arrow const& decStyle, rect_f const& refRect);
    void draw_item(element::item const& style, rect_f const& rect, utf8_string const& text);
    void draw_caret(element::caret const& style, rect_f const& refRect, point_f offset);
    auto draw_bar(element::bar const& style, rect_f const& refRect, element::bar::context const& barCtx) -> rect_f;
    auto draw_thumb(element::thumb const& style, rect_f const& refRect, element::thumb::context const& thumbCtx) -> rect_f;
    auto draw_scrollbar(element::scrollbar const& style, element::thumb const& thumbStyle, rect_f const& refRect, element::bar::context const& barCtx) -> element::scrollbar::rects;
    void draw_shadow(element::shadow const& style, rect_f const& rect, bool isCircle, element::border const& borderStyle);

    auto canvas() -> gfx::canvas&;

    auto format_text(element::text const& style, rect_f const& rect, utf8_string_view text) -> text_formatter::result;

private:
    void draw_nine_patch(nine_patch const& np, rect_f const& rect, element::border const& borderStyle);
    void draw_bordered_rect(rect_f const& rect, ui_paint const& back, element::border const& borderStyle);
    void draw_bordered_circle(rect_f const& rect, ui_paint const& back, element::border const& borderStyle);
    void draw_border(rect_f const& rect, element::border const& borderStyle, f32 borderSize, f32 borderRadius);

    auto get_paint(ui_paint const& p, rect_f const& rect) -> canvas_paint;

    auto format_text(element::text const& style, rect_f const& rect, utf8_string_view text, u32 fontSize, bool resize) -> text_formatter::result;
    auto transform_text(element::text::transform xform, utf8_string_view text) const -> utf8_string;

    gfx::canvas&       _canvas;
    std::stack<rect_f> _scissorStack;
};

////////////////////////////////////////////////////////////

class TCOB_API scissor_guard final {
public:
    scissor_guard(widget_painter& painter, widget* w);
    ~scissor_guard();

private:
    widget_painter& _painter;
};

}
