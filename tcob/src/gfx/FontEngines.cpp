// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "FontEngines.hpp"

#include <cassert>

////////////////////////////////////////////////////////////

#if defined(TCOB_ENABLE_TTF_FREETYPE)

namespace tcob::gfx::detail {

ft_ttf_font_engine::~ft_ttf_font_engine()
{
    if (_face) {
        FT_Done_Face(_face);
    }
}

auto ft_ttf_font_engine::get_name() -> string
{
    return "FREETYPE";
}

static FT_Library library {nullptr};

auto ft_ttf_font_engine::load_data(std::span<ubyte const> data, u32 fontsize) -> std::optional<font::info>
{
    _fontSize = fontsize;
    _glyphIndices.clear();

    if (!FT_New_Memory_Face(library, data.data(), static_cast<FT_Long>(data.size()), 0, &_face)) {
        FT_Set_Pixel_Sizes(_face, fontsize, fontsize);
        FT_Select_Charmap(_face, FT_ENCODING_UNICODE);

        _info = {.Ascender   = _face->size->metrics.ascender / 64.0f,
                 .Descender  = _face->size->metrics.descender / 64.0f,
                 .LineHeight = _face->size->metrics.height / 64.0f};

        return _info;
    }

    return std::nullopt;
}

auto ft_ttf_font_engine::get_kerning(u32 cp0, u32 cp1) -> f32
{
    assert(_face);
    if (!FT_HAS_KERNING(_face)) {
        return 0;
    }

    FT_Vector kerning;
    FT_Get_Kerning(_face, codepoint_to_glyphindex(cp0), codepoint_to_glyphindex(cp1), FT_KERNING_DEFAULT, &kerning);
    return kerning.x / 64.0f;
}

auto ft_ttf_font_engine::get_glyph(u32 cp) -> glyph_bitmap
{
    assert(_face);
    glyph_bitmap retValue {};

    FT_Load_Glyph(_face, codepoint_to_glyphindex(cp), 0);
    FT_Render_Glyph(_face->glyph, FT_RENDER_MODE_NORMAL);

    retValue.Glyph.Size.Width  = _face->glyph->bitmap.width;
    retValue.Glyph.Size.Height = _face->glyph->bitmap.rows;
    retValue.Glyph.Offset.X    = _face->glyph->metrics.horiBearingX / 64.0f;
    retValue.Glyph.Offset.Y    = -_face->glyph->metrics.horiBearingY / 64.0f + _info.Ascender;
    retValue.Glyph.AdvanceX    = _face->glyph->metrics.horiAdvance / 64.0f;

    retValue.Bitmap = std::vector<ubyte> {_face->glyph->bitmap.buffer, _face->glyph->bitmap.buffer + (retValue.Glyph.Size.Width * retValue.Glyph.Size.Height)};

    return retValue;
}

auto ft_ttf_font_engine::codepoint_to_glyphindex(u32 cp) -> u32
{
    assert(_face);
    auto it {_glyphIndices.find(cp)};
    if (it == _glyphIndices.end()) {
        return _glyphIndices[cp] = FT_Get_Char_Index(_face, cp);
    }

    return it->second;
}

auto ft_ttf_font_engine::Init() -> bool
{
    return !FT_Init_FreeType(&library);
}

void ft_ttf_font_engine::Done()
{
    FT_Done_FreeType(library);
}

}

#endif

////////////////////////////////////////////////////////////

#if defined(TCOB_ENABLE_TTF_STBTT)

    #include <stb/stb_truetype.h>

namespace tcob::gfx::detail {

stb_ttf_font_engine::stb_ttf_font_engine()
    : _handle {new stbtt_fontinfo}
{
}

stb_ttf_font_engine::~stb_ttf_font_engine()
{
    if (_handle) {
        delete _handle;
        _handle = nullptr;
    }
}

auto tcob::gfx::detail::stb_ttf_font_engine::get_name() -> string
{
    return "STBTT";
}

auto stb_ttf_font_engine::load_data(std::span<ubyte const> data, u32 fontsize) -> std::optional<font::info>
{
    assert(_handle);
    if (stbtt_InitFont(_handle, data.data(), stbtt_GetFontOffsetForIndex(data.data(), 0))) {
        _fontScale = stbtt_ScaleForMappingEmToPixels(_handle, static_cast<f32>(fontsize));
        _glyphIndices.clear();

        i32 ascent {0}, descent {0}, linegap {0};
        stbtt_GetFontVMetrics(_handle, &ascent, &descent, &linegap);

        _info = {.Ascender   = ascent * _fontScale,
                 .Descender  = descent * _fontScale,
                 .LineHeight = (ascent - descent + linegap) * _fontScale};
        return _info;
    }

    return std::nullopt;
}

auto stb_ttf_font_engine::get_kerning(u32 cp0, u32 cp1) -> f32
{
    assert(_handle);
    return stbtt_GetGlyphKernAdvance(_handle, codepoint_to_glyphindex(cp0), codepoint_to_glyphindex(cp1)) * _fontScale;
}

auto stb_ttf_font_engine::get_glyph(u32 cp) -> glyph_bitmap
{
    assert(_handle);
    glyph_bitmap retValue {};

    i32        glWidth {0}, glHeight {0}, xoff {0}, yoff {0};
    auto const gi {codepoint_to_glyphindex(cp)};
    auto*      data {stbtt_GetGlyphBitmap(_handle, _fontScale, _fontScale, gi, &glWidth, &glHeight, &xoff, &yoff)};
    retValue.Bitmap = std::vector<ubyte> {data, data + (glWidth * glHeight)};
    stbtt_FreeBitmap(data, nullptr);

    retValue.Glyph.Size.Width  = glWidth;
    retValue.Glyph.Size.Height = glHeight;

    i32 advanceWidth {}, lsb {};
    stbtt_GetGlyphHMetrics(_handle, gi, &advanceWidth, &lsb);
    retValue.Glyph.Offset.X = lsb * _fontScale;
    retValue.Glyph.Offset.Y = static_cast<f32>(yoff + _info.Ascender);
    retValue.Glyph.AdvanceX = advanceWidth * _fontScale;

    return retValue;
}

auto stb_ttf_font_engine::codepoint_to_glyphindex(u32 cp) -> u32
{
    assert(_handle);
    auto it {_glyphIndices.find(cp)};
    if (it == _glyphIndices.end()) {
        return _glyphIndices[cp] = static_cast<u32>(stbtt_FindGlyphIndex(_handle, cp));
    }

    return it->second;
}

}

#endif

////////////////////////////////////////////////////////////

#if defined(TCOB_ENABLE_TTF_LIBSCHRIFT)

    #include <libschrift/schrift.h>

namespace tcob::gfx::detail {

libschrift_ttf_font_engine::libschrift_ttf_font_engine()
    : _handle {new SFT}
{
}

libschrift_ttf_font_engine::~libschrift_ttf_font_engine()
{
    if (_handle) {
        sft_freefont(_handle->font);
        delete _handle;
        _handle = nullptr;
    }
}

auto tcob::gfx::detail::libschrift_ttf_font_engine::get_name() -> string
{
    return "LIBSCHRIFT";
}

auto libschrift_ttf_font_engine::load_data(std::span<ubyte const> data, u32 fontsize) -> std::optional<font::info>
{
    assert(_handle);
    _handle->font = sft_loadmem(data.data(), data.size());

    if (_handle->font) {
        _glyphIndices.clear();
        _handle->flags   = SFT_DOWNWARD_Y;
        _handle->xScale  = fontsize;
        _handle->yScale  = fontsize;
        _handle->xOffset = 0;
        _handle->yOffset = 0;

        SFT_LMetrics metrics;
        sft_lmetrics(_handle, &metrics);

        _info = {.Ascender   = static_cast<f32>(metrics.ascender),
                 .Descender  = static_cast<f32>(metrics.descender),
                 .LineHeight = static_cast<f32>(metrics.ascender - metrics.descender + metrics.lineGap)};
        return _info;
    }

    return std::nullopt;
}

auto libschrift_ttf_font_engine::get_kerning(u32 cp0, u32 cp1) -> f32
{
    assert(_handle);
    SFT_Kerning kerning;
    sft_kerning(_handle, codepoint_to_glyphindex(cp0), codepoint_to_glyphindex(cp1), &kerning);
    return static_cast<f32>(kerning.xShift);
}

auto libschrift_ttf_font_engine::get_glyph(u32 cp) -> glyph_bitmap
{
    assert(_handle);
    glyph_bitmap retValue {};

    SFT_GMetrics mtx;
    auto const   gi {codepoint_to_glyphindex(cp)};
    sft_gmetrics(_handle, gi, &mtx);

    retValue.Glyph.Size.Width  = std::max(0, mtx.minWidth - 1);
    retValue.Glyph.Size.Height = std::max(0, mtx.minHeight - 1);
    retValue.Glyph.Offset.X    = static_cast<f32>(mtx.leftSideBearing);
    retValue.Glyph.Offset.Y    = static_cast<f32>(mtx.yOffset + _info.Ascender);
    retValue.Glyph.AdvanceX    = static_cast<f32>(mtx.advanceWidth);

    SFT_Image img;
    img.width  = retValue.Glyph.Size.Width;
    img.height = retValue.Glyph.Size.Height;
    retValue.Bitmap.resize(img.width * img.height);
    img.pixels = retValue.Bitmap.data();
    if (sft_render(_handle, gi, img) == -1) {
        return {};
    }

    return retValue;
}

auto libschrift_ttf_font_engine::codepoint_to_glyphindex(u32 cp) -> u32
{
    assert(_handle);
    auto it {_glyphIndices.find(cp)};
    if (it == _glyphIndices.end()) {
        SFT_Glyph gi {0};
        sft_lookup(_handle, cp, &gi);
        return _glyphIndices[cp] = static_cast<u32>(gi);
    }

    return it->second;
}

}

#endif
