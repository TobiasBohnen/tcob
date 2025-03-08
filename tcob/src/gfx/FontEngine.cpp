// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "FontEngine.hpp"

#include <cassert>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftoutln.h>

#include "tcob/gfx/Font.hpp"

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

static FT_Library library {nullptr};

truetype_font_engine::~truetype_font_engine()
{
    if (_face) {
        FT_Done_Face(_face);
    }
}

auto truetype_font_engine::load_data(std::span<ubyte const> data, u32 fontsize) -> std::optional<font::information>
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
    if (!FT_HAS_KERNING(_face)) { return 0; }

    if (!_kerningCache.contains(cp0) || !_kerningCache[cp0].contains(cp1)) {
        FT_Vector kerning;
        FT_Get_Kerning(_face, codepoint_to_glyphindex(cp0), codepoint_to_glyphindex(cp1), FT_KERNING_DEFAULT, &kerning);
        _kerningCache[cp0][cp1] = kerning.x / 64.0f;
    }

    return _kerningCache[cp0][cp1];
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

}
