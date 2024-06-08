// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_GFX_LITEHTML)

    #include <functional>
    #include <vector>

    #include "tcob/core/FlatMap.hpp"
    #include "tcob/gfx/Font.hpp"
    #include "tcob/gfx/html/HtmlDocument.hpp"

    #include <litehtml.h>
    #include <litehtml/os_types.h>
    #include <litehtml/string_id.h>

namespace tcob::gfx::html::detail {

////////////////////////////////////////////////////////////

auto to_rect(litehtml::position const& pos) -> rect_f;
auto to_color(litehtml::web_color const& col) -> color;

////////////////////////////////////////////////////////////

class container : public litehtml::document_container {
public:
    container(document& doc, document::config& config, canvas& canvas, element_painter& painter);
    virtual ~container() = default;

    auto get_document() -> document&;

    auto create_font(char const* faceName, i32 size, i32 weight, litehtml::font_style italic, u32 decoration, litehtml::font_metrics* fm) -> litehtml::uint_ptr override;
    void delete_font(litehtml::uint_ptr hFont) override;
    auto text_width(char const* text, litehtml::uint_ptr hFont) -> i32 override;
    auto pt_to_px(i32 pt) const -> i32 override;
    auto get_default_font_size() const -> i32 override;
    auto get_default_font_name() const -> char const* override;
    void load_image(char const* src, char const* baseurl, bool redraw_on_ready) override;
    void get_image_size(char const* src, char const* baseurl, litehtml::size& sz) override;
    void set_caption(char const* caption) override;
    void set_base_url(char const* base_url) override;
    void link(std::shared_ptr<litehtml::document> const& doc, litehtml::element::ptr const& el) override;
    void on_anchor_click(char const* url, litehtml::element::ptr const& el) override;
    void set_cursor(char const* cursor) override;
    void transform_text(string& text, litehtml::text_transform tt) override;
    void import_css(string& text, string const& url, string& baseurl) override;
    void set_clip(litehtml::position const& pos, litehtml::border_radiuses const& bdr_radius) override;
    void del_clip() override;
    void get_client_rect(litehtml::position& client) const override;
    auto create_element(char const* tag_name, litehtml::string_map const& attributes, std::shared_ptr<litehtml::document> const& doc) -> std::shared_ptr<litehtml::element> override;
    void get_media_features(litehtml::media_features& media) const override;
    void get_language(string& language, string& culture) const override;
    void change_language(string const& language, string const& culture);

    void draw_list_marker(litehtml::uint_ptr hdc, litehtml::list_marker const& marker) override;
    void draw_background(litehtml::uint_ptr hdc, std::vector<litehtml::background_paint> const& bg) override;
    void draw_borders(litehtml::uint_ptr hdc, litehtml::borders const& b, litehtml::position const& draw_pos, bool root) override;
    void draw_text(litehtml::uint_ptr hdc, char const* text, litehtml::uint_ptr hFont, litehtml::web_color color, litehtml::position const& pos) override;

private:
    void init_background_draw_context(background_draw_context& ctx, litehtml::background_paint const& bg);
    void init_borders(borders& brds, litehtml::borders const& b, litehtml::position const& draw_pos);

    document&         _document;
    document::config& _config;
    canvas&           _canvas;
    element_painter&  _painter;

    string                             _baseUrl;
    string                             _caption;
    flat_map<string, texture*>         _images {};
    flat_map<string, linear_gradient>  _gradients {};
    std::vector<font*>                 _fonts;
    flat_map<usize, u32>               _fontDecorations {};
    std::vector<std::function<void()>> _overlayFunctions;
    string                             _language {"en"};
    string                             _culture;
};

}

#endif
