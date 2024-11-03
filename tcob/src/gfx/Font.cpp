// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Font.hpp"

#include <optional>
#include <utility>

#include "tcob/core/Logger.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/gfx/animation/Tween.hpp"

#include "FontEngine.hpp"

using namespace std::chrono_literals;

namespace tcob::gfx {

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

////////////////////////////////////////////////////////////

constexpr i32 FONT_TEXTURE_SIZE {2048};
constexpr f32 FONT_TEXTURE_SIZE_F {static_cast<f32>(FONT_TEXTURE_SIZE)};
constexpr u32 FONT_TEXTURE_LAYERS {3};

constexpr i32 GLYPH_PADDING {4};

font::font()
    : _engine {std::make_unique<truetype_font_engine>()}
{
}

font::~font() = default;

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
    if (auto info {_engine->load_data(fontData, size)}) {
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
                glyph.AdvanceX += _engine->get_kerning(cp0, utf32text[i + 1]);
            }
        } else {
            if (_renderGlyphCache.contains(cp0)) {
                retValue.push_back(_renderGlyphCache[cp0]);
            } else {
                retValue.push_back({_engine->load_glyph(cp0)});
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
        points.push_back(curPos);
        points.push_back(p);
        curPos = p;
    };
    cb.ConicTo = [&](point_f p0, point_f p1) {
        easing::quad_bezier_curve func;
        func.StartPoint   = curPos;
        func.ControlPoint = p0;
        func.EndPoint     = p1;
        for (f32 i {0}; i <= 1.0f; i += tolerance) { points.push_back(func(i)); }
        curPos = p1;
    };
    cb.CubicTo = [&](point_f p0, point_f p1, point_f p2) {
        easing::cubic_bezier_curve func;
        func.StartPoint    = curPos;
        func.ControlPoint0 = p0;
        func.ControlPoint1 = p1;
        func.EndPoint      = p2;
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
        auto const gl {_engine->decompose_glyph(cp0, funcs)};
        funcs.Offset.X += static_cast<i32>(gl.AdvanceX);
        if (kerning && i < len - 1) {
            funcs.Offset.X += static_cast<i32>(_engine->get_kerning(cp0, utf32text[i + 1]));
        }
    }
}

auto font::Init() -> bool
{
    return truetype_font_engine::Init();
}

void font::Done()
{
    truetype_font_engine::Done();
}

auto font::get_info() const -> font::info const&
{
    return _info;
}

auto font::cache_render_glyph(u32 cp) -> bool
{
    if (!_renderGlyphCache.contains(cp)) {
        auto       gb {_engine->render_glyph(cp)};
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

}
