// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Font.hpp"

#include <optional>
#include <utility>

#include "tcob/core/Logger.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/tweening/Tween.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftoutln.h>

using namespace std::chrono_literals;

namespace tcob::gfx {

extern "C" {
auto static move_to(FT_Vector const* to, void* user) -> i32
{
    auto* funcs {reinterpret_cast<decompose_callbacks*>(user)};
    funcs->MoveTo({(to->x / 64.0f) + funcs->Offset.X, (to->y / 64.0f) + funcs->Offset.Y});
    return 0;
}

auto static line_to(FT_Vector const* to, void* user) -> i32
{
    auto* funcs {reinterpret_cast<decompose_callbacks*>(user)};
    funcs->LineTo({(to->x / 64.0f) + funcs->Offset.X, (to->y / 64.0f) + funcs->Offset.Y});
    return 0;
}

auto static conic_to(FT_Vector const* control, FT_Vector const* to, void* user) -> i32
{
    auto* funcs {reinterpret_cast<decompose_callbacks*>(user)};
    funcs->ConicTo({(control->x / 64.0f) + funcs->Offset.X, (control->y / 64.0f) + funcs->Offset.Y},
                   {(to->x / 64.0f) + funcs->Offset.X, (to->y / 64.0f) + funcs->Offset.Y});
    return 0;
}

auto static cubic_to(FT_Vector const* control1, FT_Vector const* control2, FT_Vector const* to, void* user) -> i32
{
    auto* funcs {reinterpret_cast<decompose_callbacks*>(user)};
    funcs->CubicTo({(control1->x / 64.0f) + funcs->Offset.X, (control1->y / 64.0f) + funcs->Offset.Y},
                   {(control2->x / 64.0f) + funcs->Offset.X, (control2->y / 64.0f) + funcs->Offset.Y},
                   {(to->x / 64.0f) + funcs->Offset.X, (to->y / 64.0f) + funcs->Offset.Y});
    return 0;
}
}

// based on: https://github.com/brofield/simpleini/blob/master/ConvertUTF.c
auto static convert_UTF8_to_UTF32(string_view text) -> std::u32string
{
    static constexpr std::array<ubyte, 256> trailingBytesForUTF8 {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5};

    static constexpr std::array<u32, 6> offsetsFromUTF8 {0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL};

    usize const    texSize {text.size()};
    std::u32string retValue {};
    retValue.reserve(texSize);

    for (usize i {0}; i < texSize;) {
        ubyte const source {static_cast<ubyte>(text[i])};
        u32         ch {0};

        ubyte const extraBytesToRead {trailingBytesForUTF8[source]};
        if (i + extraBytesToRead >= texSize) {
            return {};
        }

        for (ubyte j {extraBytesToRead}; j > 0; --j) {
            ch += static_cast<ubyte>(text[i++]);
            ch <<= 6;
        }
        ch += static_cast<ubyte>(text[i++]);
        ch -= offsetsFromUTF8[extraBytesToRead];

        if (ch <= 0x0010FFFF) {
            retValue.append(1, static_cast<char32_t>(ch));
        } else {
            return {};
        }
    }

    return retValue;
}

////////////////////////////////////////////////////////////

static FT_Library library {nullptr};

truetype_font_engine::~truetype_font_engine()
{
    if (_face) {
        FT_Done_Face(_face);
    }
}

auto truetype_font_engine::load_data(std::span<ubyte const> data, u32 fontsize) -> std::optional<font_info>
{
    assert(library);

    _fontSize = fontsize;
    _glyphIndices.clear();

    if (!FT_New_Memory_Face(library, data.data(), static_cast<FT_Long>(data.size()), 0, &_face)) {
        FT_Set_Pixel_Sizes(_face, _fontSize, _fontSize);
        FT_Select_Charmap(_face, FT_ENCODING_UNICODE);

        _info = {.Ascender   = _face->size->metrics.ascender / 64.0f,
                 .Descender  = _face->size->metrics.descender / 64.0f,
                 .LineHeight = _face->size->metrics.height / 64.0f};

        return _info;
    }

    return std::nullopt;
}

auto truetype_font_engine::get_kerning(u32 cp0, u32 cp1) -> f32
{
    assert(_face);
    assert(library);
    if (!FT_HAS_KERNING(_face)) {
        return 0;
    }

    FT_Vector kerning;
    FT_Get_Kerning(_face, codepoint_to_glyphindex(cp0), codepoint_to_glyphindex(cp1), FT_KERNING_DEFAULT, &kerning);
    return kerning.x / 64.0f;
}

auto truetype_font_engine::render_glyph(u32 cp) -> std::pair<glyph, glyph_bitmap>
{
    assert(_face);
    assert(library);
    std::pair<glyph, glyph_bitmap> retValue {};

    retValue.first = load_glyph(cp);
    FT_Render_Glyph(_face->glyph, FT_RENDER_MODE_NORMAL);

    /*
        FT_Render_Glyph(_face->glyph, FT_RENDER_MODE_SDF); // render twice to force bsdf
        std::vector<byte> bitmap {_face->glyph->bitmap.buffer, _face->glyph->bitmap.buffer + (_face->glyph->bitmap.width * _face->glyph->bitmap.rows)};
        retValue.Bitmap.reserve(bitmap.size());
        for (ubyte pixel : bitmap) {
            retValue.Bitmap.push_back(pixel < 128 ? static_cast<ubyte>(256 - (128 - pixel) * 2) : 255);
        }
    */

    retValue.second.Bitmap = std::vector<ubyte> {_face->glyph->bitmap.buffer, _face->glyph->bitmap.buffer + (_face->glyph->bitmap.width * _face->glyph->bitmap.rows)};

    retValue.second.BitmapSize.Width  = _face->glyph->bitmap.width;
    retValue.second.BitmapSize.Height = _face->glyph->bitmap.rows;

    return retValue;
}

auto truetype_font_engine::decompose_glyph(u32 cp, decompose_callbacks& funcs) -> glyph
{
    auto const retValue {load_glyph(cp)};

    FT_Outline_Funcs ftFuncs = {
        .move_to  = &move_to,
        .line_to  = &line_to,
        .conic_to = &conic_to,
        .cubic_to = &cubic_to,
        .shift    = 0,
        .delta    = 0,
    };

    FT_Outline& outline {_face->glyph->outline};

    FT_Matrix matrix;
    matrix.xx = 1L << 16;
    matrix.xy = 0L << 16;
    matrix.yx = 0L << 16;
    matrix.yy = -1L << 16;
    FT_Outline_Transform(&outline, &matrix);

    FT_Outline_Decompose(&outline, &ftFuncs, reinterpret_cast<void*>(&funcs));

    return retValue;
}

auto truetype_font_engine::load_glyph(u32 cp) -> glyph
{
    assert(_face);
    assert(library);
    glyph retValue {};

    FT_Load_Glyph(_face, codepoint_to_glyphindex(cp), 0);

    retValue.Size.Width  = _face->glyph->metrics.width / 64;
    retValue.Size.Height = _face->glyph->metrics.height / 64;
    retValue.Offset.X    = _face->glyph->metrics.horiBearingX / 64.0f;
    retValue.Offset.Y    = -_face->glyph->metrics.horiBearingY / 64.0f + _info.Ascender;
    retValue.AdvanceX    = _face->glyph->metrics.horiAdvance / 64.0f;

    return retValue;
}

auto truetype_font_engine::codepoint_to_glyphindex(u32 cp) -> u32
{
    assert(_face);
    assert(library);
    auto it {_glyphIndices.find(cp)};
    if (it == _glyphIndices.end()) {
        return _glyphIndices[cp] = FT_Get_Char_Index(_face, cp);
    }

    return it->second;
}

auto truetype_font_engine::Init() -> bool
{
    return !FT_Init_FreeType(&library);
}

void truetype_font_engine::Done()
{
    FT_Done_FreeType(library);
}

////////////////////////////////////////////////////////////

constexpr i32 FONT_TEXTURE_SIZE {2048};
constexpr f32 FONT_TEXTURE_SIZE_F {static_cast<f32>(FONT_TEXTURE_SIZE)};
constexpr u32 FONT_TEXTURE_LAYERS {3};

constexpr i32 GLYPH_PADDING {4};

font::font() = default;

auto font::get_texture() const -> assets::asset_ptr<texture>
{
    return _texture;
}

auto font::load(path const& file, u32 size) noexcept -> load_status
{
    io::ifstream fs {file};
    return load(fs, size);
}

auto font::load(io::istream& stream, u32 size) noexcept -> load_status
{
    if (!stream) { return load_status::Error; }

    _fontData = stream.read_all<ubyte>();
    return load(_fontData, size);
}

auto font::load(std::span<ubyte const> fontData, u32 size) noexcept -> load_status
{
    if (auto info {_engine.load_data(fontData, size)}) {
        _info = *info;
        _renderGlyphCache.clear();
        _textureNeedsSetup = true;
        return load_status::Ok;
    }

    return load_status::Error;
}

void font::setup_texture()
{
    auto texture {get_texture()};
    texture->create({FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE}, FONT_TEXTURE_LAYERS, texture::format::R8);
    texture->Filtering = texture::filtering::Linear;
    texture->Wrapping  = texture::wrapping::ClampToBorder;

    _fontTextureCursor = {0, 0};
    _fontTextureLayer  = 0;

    _textureNeedsSetup = false;
}

auto font::render_text(utf8_string_view text, bool kerning, bool readOnlyCache) -> std::vector<rendered_glyph>
{
    if (_textureNeedsSetup && !readOnlyCache) {
        setup_texture();
    }

    auto const  utf32text {convert_UTF8_to_UTF32(text)};
    usize const len {utf32text.size()};

    std::vector<rendered_glyph> retValue;
    retValue.reserve(len);

    for (u32 i {0}; i < len; ++i) {
        u32 const cp0 {utf32text[i]};
        if (!readOnlyCache) {
            if (!cache_render_glyph(cp0)) {
                logger::Error("TrueTypeFont: shaping of text \"{}\" failed.", text);
                return {};
            }

            auto& glyph {retValue.emplace_back(_renderGlyphCache[cp0])};
            if (kerning && i < len - 1) {
                glyph.AdvanceX += _engine.get_kerning(cp0, utf32text[i + 1]);
            }
        } else {
            if (_renderGlyphCache.contains(cp0)) {
                retValue.push_back(_renderGlyphCache[cp0]);
            } else {
                retValue.push_back({_engine.load_glyph(cp0)});
            }
        }
    }

    return retValue;
}

auto font::polygonize_text(utf8_string_view text, bool kerning) -> std::vector<polygon>
{
    std::vector<polygon> retValue;

    f32 constexpr tolerance {0.05f};
    point_f              curPos;
    std::vector<point_f> points;

    auto const addPoly {[&] {
        if (!points.empty()) {
            std::ranges::reverse(points);
            auto const winding {polygons::get_winding(points)};
            if (winding == winding::CCW) {
                retValue.emplace_back().Outline = points;
            } else {
                retValue.at(retValue.size() - 1).Holes.push_back(points);
            }

            points.clear();
        }
    }};

    decompose_callbacks cb {};
    cb.MoveTo = [&](point_f p) {
        curPos = p;
        addPoly();
    };
    cb.LineTo = [&](point_f p) {
        tweening::func::linear<point_f> func;
        func.StartValue = curPos;
        func.EndValue   = p;
        for (f32 i {0}; i <= 1.0f; i += tolerance) { points.push_back(func(i)); }
        curPos = p;
    };
    cb.ConicTo = [&](point_f p0, point_f p1) {
        tweening::func::quad_bezier_curve func;
        func.Begin        = curPos;
        func.ControlPoint = p0;
        func.End          = p1;
        for (f32 i {0}; i <= 1.0f; i += tolerance) { points.push_back(func(i)); }
        curPos = p1;
    };
    cb.CubicTo = [&](point_f p0, point_f p1, point_f p2) {
        tweening::func::cubic_bezier_curve func;
        func.Begin         = curPos;
        func.ControlPoint0 = p0;
        func.ControlPoint1 = p1;
        func.End           = p2;
        for (f32 i {0}; i <= 1.0f; i += tolerance) { points.push_back(func(i)); }
        curPos = p2;
    };

    decompose_text(text, kerning, cb);
    addPoly();
    return retValue;
}

void font::decompose_text(utf8_string_view text, bool kerning, decompose_callbacks& funcs)
{
    auto const  utf32text {convert_UTF8_to_UTF32(text)};
    usize const len {utf32text.size()};

    funcs.Offset.Y += _info.Ascender;
    for (u32 i {0}; i < len; ++i) {
        u32 const  cp0 {utf32text[i]};
        auto const gl {_engine.decompose_glyph(cp0, funcs)};
        funcs.Offset.X += static_cast<i32>(gl.AdvanceX);
        if (kerning && i < len - 1) {
            funcs.Offset.X += static_cast<i32>(_engine.get_kerning(cp0, utf32text[i + 1]));
        }
    }
}

auto font::get_info() const -> font_info const&
{
    return _info;
}

auto font::cache_render_glyph(u32 cp) -> bool
{
    if (!_renderGlyphCache.contains(cp)) {
        auto       gb {_engine.render_glyph(cp)};
        auto const bitmapSize {gb.second.BitmapSize};
        if (bitmapSize.Width < 0 || bitmapSize.Height < 0) {
            return false;
        }

        // check font texture space
        if (_fontTextureCursor.X + bitmapSize.Width >= FONT_TEXTURE_SIZE) { // new line
            _fontTextureCursor.X = 0;
            _fontTextureCursor.Y += static_cast<i32>(get_info().LineHeight) + GLYPH_PADDING;
        }

        if (_fontTextureCursor.Y + bitmapSize.Height >= FONT_TEXTURE_SIZE) { // new level
            _fontTextureLayer++;
            if (_fontTextureLayer >= FONT_TEXTURE_LAYERS) {
                logger::Error("TrueTypeFont: font texture layer {} exceeds maximum.", _fontTextureLayer);
            }
            _fontTextureCursor = point_i::Zero;
        }

        if (Render.get_slot_count() > 0) {
            std::span<ubyte> pix {gb.second.Bitmap};
            Render(pix);
        }

        // write to texture
        get_texture()->update_data(_fontTextureCursor, bitmapSize, gb.second.Bitmap.data(), _fontTextureLayer, bitmapSize.Width, 1);

        // create glyph
        rendered_glyph gl {gb.first};
        gl.TexRegion = {.UVRect = {_fontTextureCursor.X / FONT_TEXTURE_SIZE_F, _fontTextureCursor.Y / FONT_TEXTURE_SIZE_F,
                                   bitmapSize.Width / FONT_TEXTURE_SIZE_F, bitmapSize.Height / FONT_TEXTURE_SIZE_F},
                        .Level  = _fontTextureLayer};

        _renderGlyphCache[cp] = gl;

        // advance cursor
        _fontTextureCursor.X += bitmapSize.Width + GLYPH_PADDING;
    }
    return true;
}

////////////////////////////////////////////////////////////

font_family::font_family(string name)
    : _name {std::move(name)}
{
}

void font_family::set_source(font::style style, path const& file)
{
    _fontSources[style] = file;
}

auto font_family::get_fallback_style(font::style style) const -> std::optional<font::style>
{
    font::style retValue {style};

    if (has_style(retValue)) { return retValue; }

    auto const ascendWhile {[&]() {
        while (static_cast<i32>(retValue.Weight) <= static_cast<i32>(font::weight::Heavy)) {
            retValue.Weight = static_cast<font::weight>(static_cast<i32>(retValue.Weight) + 100);
            if (has_style(retValue)) { return true; }
        }
        return false;
    }};

    auto const descendWhile {[&]() {
        while (static_cast<i32>(retValue.Weight) > 0) {
            retValue.Weight = static_cast<font::weight>(static_cast<i32>(retValue.Weight) - 100);
            if (has_style(retValue)) { return true; }
        }
        return false;
    }};

    // rule from https://developer.mozilla.org/en-US/docs/Web/CSS/font-weight#fallback_weights
    // If the target weight given is between 400 and 500 inclusive:
    if (style.Weight == font::weight::Normal || style.Weight == font::weight::Medium) {
        // Look for available weights between the target and 500, in ascending order.
        retValue.Weight = font::weight::Medium;
        if (has_style(retValue)) { return retValue; }

        // If no match is found, look for available weights less than the target, in descending order.
        retValue.Weight = font::weight::Normal;
        if (has_style(retValue)) { return retValue; }
        if (descendWhile()) { return retValue; }
        // If no match is found, look for available weights greater than 500, in ascending order.
        retValue.Weight = font::weight::Medium;
        if (ascendWhile()) { return retValue; }

        return std::nullopt;
    }

    // If a weight less than 400 is given,
    if (static_cast<i32>(retValue.Weight) < 400) {
        // look for available weights less than the target, in descending order.
        if (descendWhile()) { return retValue; }
        // If no match is found, look for available weights greater than the target, in ascending order.
        retValue.Weight = style.Weight;
        if (ascendWhile()) { return retValue; }
    }

    // If a weight greater than 500 is given,
    if (static_cast<i32>(retValue.Weight) > 500) {
        // look for available weights greater than the target, in ascending order.
        if (ascendWhile()) { return retValue; }
        // If no match is found, look for available weights less than the target, in descending order.
        retValue.Weight = style.Weight;
        if (descendWhile()) { return retValue; }
    }

    return std::nullopt;
}

auto font_family::has_style(font::style style) const -> bool
{
    return _fontSources.contains(style);
}

auto font_family::get_name() const -> string const&
{
    return _name;
}

auto font_family::get_font(font::style style, u32 size) -> assets::asset_ptr<font>
{
    auto fallbackStyle {get_fallback_style(style)};
    if (!fallbackStyle) {
#if !defined(__EMSCRIPTEN__) // TODO: fixed in llvm 19
        logger::Error("FontFamily {}: no sources for font style: style {}, size {}.", _name, style, size);
#endif
        return {};
    }

    auto const fontStyle {*fallbackStyle};

    // check if asset already exists
    if (auto const styleIt {_fontAssets.find(fontStyle)}; styleIt != _fontAssets.end()) {
        if (auto const sizeIt {styleIt->second.find(size)}; sizeIt != styleIt->second.end()) {
            return sizeIt->second;
        }
    }

#if !defined(__EMSCRIPTEN__) // TODO: fixed in llvm 19
    logger::Info("FontFamily {}: created new font: style {}, size {}.", _name, fontStyle, size);
#endif

    // check if font data has already been loaded
    if (!_fontData.contains(fontStyle)) {
        io::ifstream fs {_fontSources[fontStyle]};
        _fontData[fontStyle] = fs.read_all<ubyte>();
    }

    // load font
    auto const& asset {_fontAssets[fontStyle][size]};
    if (asset.get_ptr()->load(_fontData[fontStyle], size) == load_status::Ok) {
        return asset;
    }

    return {};
}

void font_family::clear_assets()
{
    _fontAssets.clear();
}

auto font_family::get_font_count() const -> isize
{
    isize retValue {0};
    for (auto const& a : _fontAssets) {
        retValue += std::ssize(a.second);
    }
    return retValue;
}

auto font_family::get_images() const -> std::vector<image>
{
    std::vector<image> retValue;
    for (auto const& f : _fontAssets) {
        for (auto const& fa : f.second) {
            fa.second->render_text("a", false, false);
            for (u32 level {0}; level < FONT_TEXTURE_LAYERS; ++level) {
                retValue.push_back(fa.second->get_texture()->copy_to_image(level));
            }
        }
    }

    return retValue;
}

void font_family::FindSources(font_family& fam, path const& source)
{
    fam._fontSources.clear();
    fam._fontData.clear();
    fam._fontAssets.clear();

    auto files {io::enumerate(io::get_parent_folder(source), {source + "*.ttf", true}, false)};
    for (auto const& file : files) {
        font::style       style;
        string_view const name {string_view {file}.substr(source.size())};

        style.IsItalic = name.find("Italic") != string::npos;

        if (name.find("Thin") != string::npos || name.find("Hairline") != string::npos) {   // Thin
            style.Weight = font::weight::Thin;
        } else if (name.find("Light") != string::npos) {
            if (name.find("Extra") != string::npos || name.find("Ultra") != string::npos) { // Light or ExtraLight
                style.Weight = font::weight::ExtraLight;
            } else {
                style.Weight = font::weight::Light;
            }
        } else if (name.find("Medium") != string::npos) { // Medium
            style.Weight = font::weight::Medium;
        } else if (name.find("Bold") != string::npos) {   // Bold or SemiBold or ExtraBold
            if (name.find("Semi") != string::npos || name.find("Demi") != string::npos) {
                style.Weight = font::weight::SemiBold;
            } else if (name.find("Extra") != string::npos || name.find("Ultra") != string::npos) {
                style.Weight = font::weight::ExtraBold;
            } else {
                style.Weight = font::weight::Bold;
            }
        } else if (name.find("Heavy") != string::npos || name.find("Black") != string::npos) { // Heavy
            style.Weight = font::weight::Heavy;
        } else {                                                                               // Normal
            style.Weight = font::weight::Normal;
        }

        fam.set_source(style, file);
    }

    fam._fontData.reserve(fam._fontSources.size());
}

void font_family::SingleFont(font_family& fam, std::span<ubyte const> font)
{
    fam._fontSources.clear();
    fam._fontData.clear();
    fam._fontAssets.clear();

    fam._fontData[{}].assign(font.begin(), font.end());
    fam._fontSources[{}] = "";
}

}
