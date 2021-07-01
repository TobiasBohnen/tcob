// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/Font.hpp>

#include <stb/stb_truetype.h>

#include <tcob/core/data/Color.hpp>
#include <tcob/core/io/FileStream.hpp>

namespace tcob {

// based on: https://github.com/brofield/simpleini/blob/master/ConvertUTF.c
auto convert_UTF8_to_UTF32(const std::string& text) -> std::u32string
{
    static const std::array<ubyte, 256> trailingBytesForUTF8 {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
    };

    static const std::array<u32, 6> offsetsFromUTF8 { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
        0x03C82080UL, 0xFA082080UL, 0x82082080UL };

    std::u32string retValue {};
    for (u32 i { 0 }; i < text.size();) {
        ubyte source { static_cast<ubyte>(text[i]) };
        u32 ch { 0 };

        auto extraBytesToRead { trailingBytesForUTF8[source] };
        if (i + extraBytesToRead >= text.size()) {
            return {};
        }

        for (u32 j { extraBytesToRead }; j > 0; j--) {
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

constexpr i32 FONT_TEXTURE_SIZE { 1024 };
constexpr u32 GLYPH_PADDING { 4 };
const std::string FONT_WARMUP { "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.,;:'!\"%&()=?<>" };

Font::Font()
    : _fontTexture { std::make_shared<gl::Texture2D>() }
    , _material { std::make_shared<Material>() }
    , _matRes { std::make_shared<Resource<Material>>(_material) }
{
    _material->Texture = std::make_shared<Resource<gl::Texture>>(_fontTexture);
}

Font::~Font() = default;

auto Font::material() const -> ResourcePtr<Material>
{
    if (_matRes) {
        if (!_matRes->Shader) {
            _matRes->Shader = DefaultShader;
        }
    }

    return _matRes;
}

void Font::material(ResourcePtr<Material> material)
{
    _matRes = std::move(material);

    if (_matRes) {
        _matRes->Texture = std::make_shared<Resource<gl::Texture>>(_fontTexture);
        _material = _matRes.get()->object_ptr();
    }
}

auto Font::texture() const -> gl::Texture2D*
{
    return _fontTexture.get();
}

auto Font::info() const -> FontInfo
{
    f32 lg { _lineGapOverride.has_value() ? _lineGapOverride.value() : linegap() };
    return {
        .Ascender = ascender(),
        .Descender = descender(),
        .Linegap = lg,
        .Height = ascender() - descender() + lg
    };
}

void Font::create_texture()
{
    _fontTexture->create_or_resize({ FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE }, tcob::gl::TextureFormat::R8);
    _fontTexture->filtering(gl::TextureFiltering::Linear);
}

auto Font::kerning() const -> bool
{
    return _kerning;
}

void Font::kerning(bool kerning)
{
    if (_kerning != kerning) {
        _kerning = kerning;
        Changed();
    }
}

void Font::line_gap_override(f32 val)
{
    if (_lineGapOverride != val) {
        _lineGapOverride = val;
        Changed();
    }
}

////////////////////////////////////////////////////////////

TrueTypeFont::TrueTypeFont(bool sdfmode)
    : Font {}
    , _fontInfo { new stbtt_fontinfo }
    , _sdfMode(sdfmode)
{
}

TrueTypeFont::~TrueTypeFont()
{
    if (_fontInfo) {
        delete _fontInfo;
        _fontInfo = nullptr;
    }
}

auto TrueTypeFont::load(const std::string& filename, u32 fontSize) -> bool
{
    if (!FileSystem::exists(filename)) {
        //TODO: log error
        return false;
    }

    InputFileStreamU fs { filename };
    _fontData = fs.read_all();

    if (!stbtt_InitFont(_fontInfo, _fontData.data(), stbtt_GetFontOffsetForIndex(_fontData.data(), 0)))
        return false;

    _fontScale = stbtt_ScaleForPixelHeight(_fontInfo, static_cast<f32>(fontSize));
    _fontSize = fontSize;

    _glyphs.clear();
    _glyphIndices.clear();

    create_texture();
    std::array<u8, 9> c {};
    c.fill(0xff);
    texture()->update(PointU::Zero, { 3, 3 }, &c, 3, 1);
    _fontTextureCursor = { 4, 0 };

    stbtt_GetFontVMetrics(_fontInfo, &_ascent, &_descent, &_lineGap);

    shape_text(FONT_WARMUP);

    return true;
}

auto TrueTypeFont::shape_text(const std::string& text) -> std::vector<Glyph>
{
    const auto utf32text { convert_UTF8_to_UTF32(text) };
    const isize len { utf32text.size() };
    std::vector<Glyph> retValue;
    retValue.reserve(len);

    for (u32 i { 0 }; i < len; ++i) {
        const u32 gi { codepoint_to_glyphindex(utf32text[i]) };
        if (!cache_glyph(gi)) {
            return {};
        }

        auto glyph { _glyphs[gi] }; //copy glyph
        if (kerning() && i < len - 1) {
            glyph.Advance += stbtt_GetGlyphKernAdvance(_fontInfo, gi, utf32text[i + 1]) * _fontScale;
        }
        retValue.push_back(glyph);
    }

    return retValue;
}

auto TrueTypeFont::ascender() const -> f32
{
    return _ascent * _fontScale;
}

auto TrueTypeFont::descender() const -> f32
{
    return _descent * _fontScale;
}

auto TrueTypeFont::linegap() const -> f32
{
    return _lineGap * _fontScale;
}

auto TrueTypeFont::cache_glyph(u32 gi) -> bool
{
    if (!_glyphs.contains(gi)) {

        i32 glWidth { 0 }, glHeight { 0 }, xoff { 0 }, yoff { 0 };
        ubyte* bitmap;
        if (_sdfMode) {
            const u8 onEdgeValue { 0x7F };
            const f32 pixelDistScale { 0x3F };
            bitmap = stbtt_GetGlyphSDF(_fontInfo, _fontScale, gi, GLYPH_PADDING, onEdgeValue, pixelDistScale, &glWidth, &glHeight, &xoff, &yoff);
        } else {
            bitmap = stbtt_GetGlyphBitmap(_fontInfo, _fontScale, _fontScale, gi, &glWidth, &glHeight, &xoff, &yoff);
        }

        // check font texture space
        if (_fontTextureCursor.X + glWidth >= FONT_TEXTURE_SIZE) { //new line
            _fontTextureCursor.X = 0;
            _fontTextureCursor.Y += static_cast<u32>(info().Height) + GLYPH_PADDING;
        }

        if (_fontTextureCursor.Y + glHeight >= FONT_TEXTURE_SIZE) { //new level
            // TODO: resize texture
            _fontTextureCursor = PointU::Zero;
        }

        // write to texture
        auto [x, y] { _fontTextureCursor };
        texture()->update({ x, y }, { static_cast<u32>(glWidth), static_cast<u32>(glHeight) }, bitmap, glWidth, 1);

        if (_sdfMode)
            stbtt_FreeSDF(bitmap, nullptr);
        else
            stbtt_FreeBitmap(bitmap, nullptr);

        i32 advanceWidth {}, lsb {};
        stbtt_GetGlyphHMetrics(_fontInfo, gi, &advanceWidth, &lsb);

        // create glyph
        Glyph glyph {
            .Bearing = { static_cast<f32>(xoff), static_cast<f32>(yoff) },
            .Size = { static_cast<f32>(glWidth), static_cast<f32>(glHeight) },
            .Offset = lsb * _fontScale,
            .Advance = advanceWidth * _fontScale,
            .UVRect = { static_cast<f32>(x) / FONT_TEXTURE_SIZE, static_cast<f32>(y) / FONT_TEXTURE_SIZE,
                static_cast<f32>(glWidth) / FONT_TEXTURE_SIZE, static_cast<f32>(glHeight) / FONT_TEXTURE_SIZE }
        };
        _glyphs[gi] = glyph;

        // advance cursor
        _fontTextureCursor.X += glWidth + GLYPH_PADDING;
    }
    return true;
}

auto TrueTypeFont::codepoint_to_glyphindex(u32 codepoint) -> u32
{
    if (!_glyphIndices.contains(codepoint)) {
        _glyphIndices[codepoint] = static_cast<u32>(stbtt_FindGlyphIndex(_fontInfo, codepoint));
    }

    return _glyphIndices[codepoint];
}
}