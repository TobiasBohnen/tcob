// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "FontEngines.hpp"

#include <cassert>

namespace tcob::gfx::detail {

ft_ttf_font_engine::~ft_ttf_font_engine()
{
    if (_face) {
        FT_Done_Face(_face);
    }
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
