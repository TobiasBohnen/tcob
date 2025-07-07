// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <stack>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/TextFormatter.hpp"
#include "tcob/gfx/Transform.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API widget_painter {
public:
    using overlay_func = std::function<void(widget_painter&)>;

    explicit widget_painter(gfx::canvas& canvas);

    void begin(f32 alpha);
    void begin(f32 alpha, gfx::transform const& xform);
    void end();
    void push_scissor(rect_f const& globalScissor);
    void pop_scissor();

    void add_overlay(overlay_func const& func);
    void draw_overlays();

    void draw_background_and_border(widget_style const& style, rect_f& rect, bool isCircle);

    void draw_text(text_element const& style, rect_f const& rect, utf8_string const& text);
    void draw_text(text_element const& style, rect_f const& rect, gfx::text_formatter::result const& text);
    void draw_text_and_icon(text_element const& style, rect_f const& rect, utf8_string const& text, icon const& icon);

    void draw_tick(tick_element const& style, rect_f const& rect);
    void draw_item(item_element const& style, rect_f const& rect, item const& item);
    void draw_caret(caret_element const& style, rect_f const& rect, point_f offset);

    auto draw_bar(bar_element const& style, rect_f const& rect, bar_element::context const& barCtx) -> rect_f;
    auto draw_thumb(thumb_element const& style, rect_f const& rect, thumb_element::context const& thumbCtx) -> rect_f;
    auto draw_nav_arrow(nav_arrow_element const& style, rect_f const& rect, bool up) -> rect_f;

    auto canvas() -> gfx::canvas&;

    auto format_text(text_element const& style, rect_f const& rect, utf8_string_view text) -> gfx::text_formatter::result;

private:
    void do_nine_patch(nine_patch const& np, rect_f const& rect, border_element const& borderStyle);
    void do_bordered_rect(rect_f const& rect, ui_paint const& back, border_element const& borderStyle);
    void do_bordered_circle(rect_f const& rect, ui_paint const& back, border_element const& borderStyle);
    void do_border(rect_f const& rect, border_element const& borderStyle, f32 borderSize, f32 borderRadius);
    void do_shadow(shadow_element const& style, rect_f const& rect, bool isCircle, border_element const& borderStyle);

    auto get_paint(ui_paint const& p, rect_f const& rect) -> gfx::canvas::paint;

    auto format_text(text_element const& style, rect_f const& rect, utf8_string_view text, u32 fontSize, bool resize) -> gfx::text_formatter::result;
    auto transform_text(text_transform xform, utf8_string_view text) const -> utf8_string;

    gfx::canvas&              _canvas;
    std::stack<rect_f>        _scissorStack;
    std::vector<overlay_func> _overlays;
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
