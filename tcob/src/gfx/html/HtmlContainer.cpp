// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "HtmlContainer.hpp"

#include <memory>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/html/HtmlElementPainter.hpp"

#if defined(TCOB_ENABLE_ADDON_GFX_LITEHTML)

    #include <litehtml/css_length.h>
    #include <litehtml/el_space.h>
    #include <litehtml/el_text.h>
    #include <litehtml/html.h>
    #include <litehtml/render_item.h>

/* TODO:
TAGS:
    dialog
    datalist

INPUT TYPES:
   <input type="number">
   <input type="password">
   <input type="text"> (default value)

*/

namespace tcob::gfx::html::detail {

////////////////////////////////////////////////////////////

auto to_rect(litehtml::position const& pos) -> rect_f
{
    return {static_cast<f32>(pos.x), static_cast<f32>(pos.y),
            static_cast<f32>(pos.width), static_cast<f32>(pos.height)};
}

auto to_color(litehtml::web_color const& col) -> color
{
    return {col.red, col.green, col.blue, col.alpha};
}

auto static to_border_style(litehtml::border_style style) -> border_style
{
    // TODO:add styles
    switch (style) {
    case litehtml::border_style_solid:
        return border_style::Solid;
    default:
        return border_style::None;
    }
}

auto static parse_to_keyword(std::vector<string> const& tokens) -> degree_f
{
    std::vector<string> tokens2;
    litehtml::split_string(tokens[0], tokens2, " ");
    i32 idx0 {tokens2.size() > 1 ? litehtml::value_index(tokens2[1], "top;bottom;left;right") : -1};
    i32 idx1 {tokens2.size() > 2 ? litehtml::value_index(tokens2[2], "top;bottom;left;right") : -1};

    switch (idx0) {
    case 0:
        switch (idx1) {
        case -1: // top
            return degree_f {0};
        case 2:  // top-left
            return degree_f {315};
        case 3:  // top-right
            return degree_f {45};
        }
    case 1:
        switch (idx1) {
        case -1: // bottom
            return degree_f {180};
        case 2:  // bottom-left
            return degree_f {225};
        case 3:  // bottom-right
            return degree_f {135};
        }
    case 2:
        switch (idx1) {
        case -1: // left
            return degree_f {270};
        case 0:  // top-left
            return degree_f {315};
        case 1:  // bottom-left
            return degree_f {225};
        }
    case 3:
        switch (idx1) {
        case -1: // right
            return degree_f {90};
        case 0:  // top-right
            return degree_f {45};
        case 1:  // bottom-right
            return degree_f {135};
        }
    }

    return degree_f {180};
}

auto static parse_linear_gradient(string const& str) -> linear_gradient
{
    degree_f                angle {180};
    std::vector<color_stop> colorStops;

    std::vector<string> tokens;
    litehtml::split_string(str, tokens, ",");
    if (tokens.empty()) {
        return {};
    }

    usize i {1};
    litehtml::trim(tokens[0]);
    if (tokens[0].ends_with("deg")) {
        angle = static_cast<f32>(std::stoi(tokens[0]));
    } else if (tokens[0].ends_with("grad")) {
        angle = std::stof(tokens[0]) / 400.0f * 360.0f;
    } else if (tokens[0].ends_with("rad")) {
        angle = radian_f {std::stof(tokens[0])};
    } else if (tokens[0].ends_with("turn")) {
        angle = std::stof(tokens[0]) * 360;
    } else if (tokens[0].starts_with("to")) {
        angle = parse_to_keyword(tokens);
    } else {
        i = 0;
    }

    // TODO: only [color percentage (percentage)] atm
    for (; i < tokens.size(); ++i) {
        std::vector<string> tokens2;
        litehtml::split_string(tokens[i], tokens2, " ");
        if (tokens2.size() >= 2) {
            litehtml::trim(tokens2[0]);
            color const c {color::FromString(tokens2[0])};
            f32 const   p0 {std::stoi(tokens2[1]) / 100.0f};
            // add first color at zero
            if (colorStops.empty() && p0 > 0.0f) {
                colorStops.emplace_back(0.0f, c);
            }

            colorStops.emplace_back(p0, c);

            if (tokens2.size() == 3) {
                f32 const p1 {std::stoi(tokens2[2]) / 100.0f};
                colorStops.emplace_back(p1, c);
            }

            // add last color at one
            if (i == tokens.size() - 1 && p0 <= 1.0f) {
                colorStops.emplace_back(1.0f, c);
            }
        }
    }

    return {.Angle = angle, .Colors = color_gradient {colorStops}};
}

////////////////////////////////////////////////////////////

container::container(document& doc, document::config& config, canvas& canvas, element_painter& painter)
    : _document {doc}
    , _config(config)
    , _canvas(canvas)
    , _painter(painter)
{
}

auto container::get_document() -> document&
{
    return _document;
}

auto container::create_font(char const* faceName, i32 size, i32 weight, litehtml::font_style italic, u32 decoration, litehtml::font_metrics* fm) -> litehtml::uint_ptr
{
    static_cast<void>(faceName);

    font::style style {};
    if (italic == litehtml::font_style::font_style_italic) {
        style.IsItalic = true;
    }
    style.Weight = static_cast<font::weight>(weight);

    auto const font {_config.Fonts->get_font(style, size)};

    auto const& fontInfo {font->get_info()};
    fm->ascent      = static_cast<i32>(fontInfo.Ascender);
    fm->descent     = -static_cast<i32>(fontInfo.Descender);
    fm->height      = fm->ascent + fm->descent;
    fm->x_height    = font->shape_text("x", false, true)[0].Size.Height;
    fm->draw_spaces = true;
    _fonts.push_back(font.get_obj());
    usize const retValue {_fonts.size() - 1};
    _fontDecorations[retValue] = decoration;
    return retValue + 1;
}

void container::delete_font(litehtml::uint_ptr hFont)
{
    static_cast<void>(hFont);
    // nothing to do
}

auto container::text_width(char const* text, litehtml::uint_ptr hFont) -> i32
{
    auto* f {_fonts[hFont - 1]};
    return f
        ? static_cast<i32>(text_formatter::measure_text(text, *f, -1, true).Width)
        : -1;
}

auto container::pt_to_px(i32 pt) const -> i32
{
    // pixels = points / 72 * 96 //TODO: get DPI from SDL or window
    return static_cast<i32>(static_cast<f32>(pt) / 72.0f * 96.0f);
}

auto container::get_default_font_size() const -> i32
{
    return _config.DefaultFontSize;
}

auto container::get_default_font_name() const -> char const*
{
    return _config.Fonts->get_name().c_str();
}

void container::load_image(char const* src, char const* baseurl, bool redraw_on_ready)
{
    static_cast<void>(baseurl);

    if (!_images.contains(src)) {
        if (auto tex {_config.AssetGroup->get<texture>(src)}; tex.is_ready()) {
            _images[src] = tex.get_obj();

            if (redraw_on_ready) {
                _document.force_redraw();
            }
        } else if (!_gradients.contains(src)) {
            auto const imageSrc {helper::to_string(src)};
            auto const first {imageSrc.find('(') + 1};
            auto const last {imageSrc.find_last_of(')')};
            if (first != string::npos && last != string::npos) {
                string gradientString {imageSrc.substr(first, last - first)};
                if (imageSrc.starts_with("linear-gradient")) {
                    _gradients[src] = parse_linear_gradient(gradientString);
                }
            }

            if (redraw_on_ready) {
                _document.force_redraw();
            }
        }
    }
}

void container::get_image_size(char const* src, char const* baseurl, litehtml::size& sz)
{
    static_cast<void>(baseurl);

    if (!_images.contains(src)) {
        return;
    }

    auto* const tex {_images[src]};
    if (tex) {
        sz.height = tex->get_size().Height;
        sz.width  = tex->get_size().Width;
    }
}

void container::set_caption(char const* caption)
{
    _caption = caption;
}

void container::set_base_url(char const* base_url)
{
    _baseUrl = base_url;
}

void container::link(std::shared_ptr<litehtml::document> const& doc, litehtml::element::ptr const& el)
{ // TODO
    static_cast<void>(doc);
    static_cast<void>(el);
}

void container::on_anchor_click(char const* url, litehtml::element::ptr const& el)
{
    el->set_pseudo_class(litehtml::_id("visited"), true);
    _document.AnchorClick(url);
}

void container::set_cursor(char const* cursor)
{
    auto winCursor {_config.Window->Cursor()};
    if (winCursor.is_ready()) {
        winCursor->ActiveMode = cursor;
    }
}

void container::transform_text(string& text, litehtml::text_transform tt)
{
    switch (tt) {
    case litehtml::text_transform::text_transform_capitalize:
        text[0] = static_cast<char>(std::toupper(text[0]));
        break;
    case litehtml::text_transform::text_transform_lowercase:
        for (auto& c : text) {
            c = static_cast<char>(std::tolower(c));
        }
        break;
    case litehtml::text_transform::text_transform_uppercase:
        for (auto& c : text) {
            c = static_cast<char>(std::toupper(c));
        }
        break;
    default:
        break;
    }
}

void container::import_css(string& text, string const& url, string& baseurl)
{
    string const burl {baseurl.empty() ? _baseUrl : baseurl};
    string const path {_config.AssetGroup->get_mount_point() + burl + url};
    text = io::read_as_string(path);
}

void container::set_clip(litehtml::position const& pos, litehtml::border_radiuses const& bdr_radius)
{
    static_cast<void>(bdr_radius);
    auto& canvas {_canvas};
    canvas.set_scissor(rect_f {static_cast<f32>(pos.x),
                               static_cast<f32>(pos.y),
                               static_cast<f32>(canvas.get_size().Width - pos.x),
                               static_cast<f32>(canvas.get_size().Height - pos.y)});
}

void container::del_clip()
{
    _canvas.reset_scissor();
}

void container::get_client_rect(litehtml::position& client) const
{
    client.x      = 0;
    client.y      = 0;
    client.width  = _canvas.get_size().Width;
    client.height = _canvas.get_size().Height;
}

auto container::create_element(char const*, litehtml::string_map const&, std::shared_ptr<litehtml::document> const&) -> std::shared_ptr<litehtml::element>
{
    return {nullptr};
}

void container::get_media_features(litehtml::media_features& media) const
{
    media.type          = litehtml::media_type_screen;
    media.width         = _canvas.get_size().Width;
    media.height        = _canvas.get_size().Height;
    media.device_width  = _config.Window->Size().Width;
    media.device_height = _config.Window->Size().Height;
    media.color         = 8;
    media.monochrome    = 0;
    media.color_index   = 0;
    media.resolution    = 96; // TODO: get DPI from SDL or window
}

void container::get_language(string& language, string& culture) const
{
    language = _language;
    culture  = _culture;
}

void container::change_language(string const& language, string const& culture)
{
    _language = language;
    _culture  = culture;
}

void container::init_background_draw_context(background_draw_context& ctx, litehtml::background_paint const& bg)
{
    ctx.BackgroundColor         = to_color(bg.color);
    ctx.BorderRadii.BottomLeft  = static_cast<f32>(bg.border_radius.bottom_left_x);
    ctx.BorderRadii.TopLeft     = static_cast<f32>(bg.border_radius.top_left_x);
    ctx.BorderRadii.BottomRight = static_cast<f32>(bg.border_radius.bottom_right_x);
    ctx.BorderRadii.TopRight    = static_cast<f32>(bg.border_radius.top_right_x);
    ctx.ClipBox                 = to_rect(bg.clip_box);
    if (_images.contains(bg.image)) { // image
        ctx.Image     = _images[bg.image];
        ctx.ImageSize = {static_cast<f32>(bg.image_size.width), static_cast<f32>(bg.image_size.height)};
    } else if (_gradients.contains(bg.image)) {
        ctx.Gradient = _gradients[bg.image];
    }

    ctx.OriginBox = to_rect(bg.origin_box);
    switch (bg.repeat) {
    case litehtml::background_repeat_repeat:
        ctx.Repeat = background_repeat::Repeat;
        break;
    case litehtml::background_repeat_repeat_x:
        ctx.Repeat = background_repeat::RepeatX;
        break;
    case litehtml::background_repeat_repeat_y:
        ctx.Repeat = background_repeat::RepeatY;
        break;
    case litehtml::background_repeat_no_repeat:
        ctx.Repeat = background_repeat::NoRepeat;
        break;
    }
}

void container::init_borders(borders& brds, litehtml::borders const& b, litehtml::position const& draw_pos)
{
    rect_f const rect {to_rect(draw_pos)};
    brds.BorderRadii.BottomLeft  = static_cast<f32>(b.radius.bottom_left_x);
    brds.BorderRadii.BottomRight = static_cast<f32>(b.radius.bottom_right_x);
    brds.BorderRadii.TopLeft     = static_cast<f32>(b.radius.top_left_x);
    brds.BorderRadii.TopRight    = static_cast<f32>(b.radius.top_right_x);
    brds.Bottom.Color            = to_color(b.bottom.color);
    brds.Left.Color              = to_color(b.left.color);
    brds.Right.Color             = to_color(b.right.color);
    brds.Top.Color               = to_color(b.top.color);
    brds.Bottom.Style            = to_border_style(b.bottom.style);
    brds.Left.Style              = to_border_style(b.left.style);
    brds.Right.Style             = to_border_style(b.right.style);
    brds.Top.Style               = to_border_style(b.top.style);
    brds.Bottom.Width            = static_cast<f32>(b.bottom.width);
    brds.Left.Width              = static_cast<f32>(b.left.width);
    brds.Right.Width             = static_cast<f32>(b.right.width);
    brds.Top.Width               = static_cast<f32>(b.top.width);
    brds.DrawBox                 = rect_f::FromLTRB(rect.X + brds.Left.Width / 2, rect.Y + brds.Top.Width / 2,
                                                    rect.right() - brds.Right.Width / 2, rect.bottom() - brds.Bottom.Width / 2);
}

void container::draw_text(litehtml::uint_ptr hdc, char const* text, litehtml::uint_ptr hFont, litehtml::web_color col, litehtml::position const& pos)
{
    static_cast<void>(hdc);
    using namespace tcob::enum_ops;

    font_decorations deco {font_decorations::None};
    u32 const        decos {_fontDecorations[hFont - 1]};
    if (decos != 0) {
        if (decos & litehtml::font_decoration_linethrough) {
            deco = deco | font_decorations::Linethrough;
        }
        if (decos & litehtml::font_decoration_overline) {
            deco = deco | font_decorations::Overline;
        }
        if (decos & litehtml::font_decoration_underline) {
            deco = deco | font_decorations::Underline;
        }
    }

    _painter.draw_text({text, to_rect(pos), _fonts[hFont - 1], to_color(col), deco});
}

void container::draw_background(litehtml::uint_ptr hdc, std::vector<litehtml::background_paint> const& bg)
{
    static_cast<void>(hdc);

    background_draw_context ctx;
    init_background_draw_context(ctx, bg[0]); // TODO: support mulitple backgrounds
    _painter.draw_background(ctx);
}

void container::draw_borders(litehtml::uint_ptr hdc, litehtml::borders const& b, litehtml::position const& draw_pos, bool root)
{
    static_cast<void>(hdc);
    static_cast<void>(root);
    if (!b.is_visible()) {
        return;
    }

    borders brds;
    init_borders(brds, b, draw_pos);
    _painter.draw_borders(brds);
}

void container::draw_list_marker(litehtml::uint_ptr hdc, litehtml::list_marker const& marker)
{
    static_cast<void>(hdc);
    list_marker_draw_context ctx;
    ctx.Box   = to_rect(marker.pos);
    ctx.Color = to_color(marker.color);
    if (!marker.image.empty()) {
        ctx.Image = _images[marker.image];
        ctx.Type  = list_marker_type::Image;
    } else {
        switch (marker.marker_type) {
        case litehtml::list_style_type_circle:
            ctx.Type = list_marker_type::Circle;
            break;
        case litehtml::list_style_type_disc:
            ctx.Type = list_marker_type::Disc;
            break;
        case litehtml::list_style_type_square:
            ctx.Type = list_marker_type::Square;
            break;
        default:
            break;
        }
    }
    ctx.Index = marker.index;
    _painter.draw_list_marker(ctx);
}

}

#endif
