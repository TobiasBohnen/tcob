// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "HtmlContainer.hpp"

#include <memory>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/assets/AssetGroup.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/html/HtmlElementPainter.hpp"

#if defined(TCOB_ENABLE_ADDON_GFX_LITEHTML)

    #include <litehtml/css_length.h>
    #include <litehtml/el_space.h>
    #include <litehtml/el_text.h>
    #include <litehtml/html.h>
    #include <litehtml/render_item.h>

namespace tcob::gfx::html::detail {

////////////////////////////////////////////////////////////

auto to_rect(litehtml::position const& pos) -> rect_f
{
    return {static_cast<f32>(pos.x), static_cast<f32>(pos.y),
            static_cast<f32>(pos.width), static_cast<f32>(pos.height)};
}

auto to_point(litehtml::pointF const& pos) -> point_f
{
    return {static_cast<f32>(pos.x), static_cast<f32>(pos.y)};
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

////////////////////////////////////////////////////////////

container::container(document& doc, document::config& config, canvas& canvas, element_painter& painter)
    : _document {doc}
    , _config {config}
    , _canvas {canvas}
    , _painter {painter}
{
}

auto container::get_document() -> document&
{
    return _document;
}

void container::set_size(size_i size)
{
    _windowSize = size;
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

    auto const& fontInfo {font->info()};
    fm->ascent      = static_cast<i32>(fontInfo.Ascender);
    fm->descent     = -static_cast<i32>(fontInfo.Descender);
    fm->height      = fm->ascent + fm->descent;
    fm->x_height    = font->render_text("x", false, true)[0].Size.Height;
    fm->draw_spaces = true;
    _fonts.push_back(font.ptr());
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
        ? static_cast<i32>(text_formatter::measure(text, *f, -1, true).Width)
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
    return _config.Fonts->name().c_str();
}

void container::load_image(char const* src, char const* baseurl, bool redraw_on_ready)
{
    static_cast<void>(baseurl);

    if (!_images.contains(src)) {
        if (auto tex {_config.AssetGroup->get<texture>(src)}; tex.is_ready()) {
            _images[src] = tex.ptr();

            if (redraw_on_ready) {
                _document.force_redraw();
            }
        }
    }
}

void container::get_image_size(char const* src, char const* baseurl, litehtml::size& sz)
{
    static_cast<void>(baseurl);

    if (!_images.contains(src)) { return; }

    auto* const tex {_images[src]};
    if (tex) {
        sz.height = tex->info().Size.Height;
        sz.width  = tex->info().Size.Width;
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

void container::on_mouse_event(litehtml::element::ptr const& el, litehtml::mouse_event event)
{ // TODO
    static_cast<void>(el);
    static_cast<void>(event);
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
    string const path {_config.AssetGroup->mount_point() + burl + url};
    text = io::read_as_string(path);
}

void container::set_clip(litehtml::position const& pos, litehtml::border_radiuses const& bdr_radius)
{
    static_cast<void>(bdr_radius);
    auto& canvas {_canvas};
    canvas.set_scissor(rect_f {static_cast<f32>(pos.x),
                               static_cast<f32>(pos.y),
                               static_cast<f32>(_windowSize.Width - pos.x),
                               static_cast<f32>(_windowSize.Height - pos.y)});
}

void container::del_clip()
{
    _canvas.reset_scissor();
}

void container::get_client_rect(litehtml::position& client) const
{
    client.x      = 0;
    client.y      = 0;
    client.width  = _windowSize.Width;
    client.height = _windowSize.Height;
}

auto container::create_element(char const*, litehtml::string_map const&, std::shared_ptr<litehtml::document> const&) -> std::shared_ptr<litehtml::element>
{
    return {nullptr};
}

void container::get_media_features(litehtml::media_features& media) const
{
    media.type          = litehtml::media_type_screen;
    media.width         = _windowSize.Width;
    media.height        = _windowSize.Height;
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
    brds.DrawBox                 = rect_f::FromLTRB(rect.left() + (brds.Left.Width / 2), rect.top() + (brds.Top.Width / 2),
                                                    rect.right() - (brds.Right.Width / 2), rect.bottom() - (brds.Bottom.Width / 2));
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

void container::init_background(base_draw_context& ctx, litehtml::background_layer const& layer)
{
    ctx.BorderRadii.BottomLeft  = static_cast<f32>(layer.border_radius.bottom_left_x);
    ctx.BorderRadii.TopLeft     = static_cast<f32>(layer.border_radius.top_left_x);
    ctx.BorderRadii.BottomRight = static_cast<f32>(layer.border_radius.bottom_right_x);
    ctx.BorderRadii.TopRight    = static_cast<f32>(layer.border_radius.top_right_x);
    ctx.ClipBox                 = to_rect(layer.clip_box);

    ctx.OriginBox = to_rect(layer.origin_box);
    switch (layer.repeat) {
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

void container::draw_borders(litehtml::uint_ptr hdc, litehtml::borders const& b, litehtml::position const& draw_pos, bool root)
{
    static_cast<void>(hdc);
    static_cast<void>(root);
    if (!b.is_visible()) { return; }

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

void container::draw_image(litehtml::uint_ptr hdc, litehtml::background_layer const& layer, std::string const& url, std::string const& base_url)
{
    static_cast<void>(hdc);
    static_cast<void>(base_url);

    image_draw_context ctx;
    init_background(ctx, layer);
    if (_images.contains(url)) { // image
        ctx.Image = _images[url];
    }

    _painter.draw_image(ctx);
}

void container::draw_solid_fill(litehtml::uint_ptr hdc, litehtml::background_layer const& layer, litehtml::web_color const& color)
{
    static_cast<void>(hdc);

    solid_draw_context ctx;
    init_background(ctx, layer);
    ctx.BackgroundColor = to_color(color);

    _painter.draw_solid_color(ctx);
}

void container::draw_linear_gradient(litehtml::uint_ptr hdc, litehtml::background_layer const& layer, litehtml::background_layer::linear_gradient const& gradient)
{
    static_cast<void>(hdc);

    gradient_draw_context ctx;
    init_background(ctx, layer);

    std::vector<color_stop> colors;
    colors.reserve(gradient.color_points.size());
    for (auto const& cp : gradient.color_points) {
        colors.emplace_back(cp.offset, to_color(cp.color));
    }
    ctx.Gradient = _canvas.create_linear_gradient(to_point(gradient.start), to_point(gradient.end), color_gradient {colors});

    _painter.draw_gradient(ctx);
}

void container::draw_radial_gradient(litehtml::uint_ptr hdc, litehtml::background_layer const& layer, litehtml::background_layer::radial_gradient const& gradient)
{
    static_cast<void>(hdc);

    gradient_draw_context ctx;
    init_background(ctx, layer);

    std::vector<color_stop> colors;
    colors.reserve(gradient.color_points.size());
    for (auto const& cp : gradient.color_points) {
        colors.emplace_back(cp.offset, to_color(cp.color));
    }
    ctx.Gradient = _canvas.create_radial_gradient(to_point(gradient.position), 0, gradient.radius.x, color_gradient {colors});

    _painter.draw_gradient(ctx);
}

void container::draw_conic_gradient(litehtml::uint_ptr hdc, litehtml::background_layer const& layer, litehtml::background_layer::conic_gradient const& gradient)
{
    static_cast<void>(hdc);
    static_cast<void>(layer);
    static_cast<void>(gradient);
    // not supported
}
}

#endif
