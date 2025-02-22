// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Interfaces.hpp"
#include "tcob/gfx/Font.hpp"

using FT_Face = struct FT_FaceRec_*;

namespace tcob::gfx {

struct glyph_bitmap {
    std::vector<ubyte> Bitmap {};
    size_i             BitmapSize {};
};

class TCOB_API truetype_font_engine : public non_copyable {
public:
    truetype_font_engine() = default;
    ~truetype_font_engine();

    auto load_data(std::span<ubyte const> data, u32 fontsize) -> std::optional<font::information>;
    auto get_kerning(u32 cp0, u32 cp1) -> f32;

    auto render_glyph(u32 cp) -> std::pair<glyph, glyph_bitmap>;
    auto decompose_glyph(u32 cp, decompose_callbacks& funcs) -> glyph;
    auto load_glyph(u32 cp) -> glyph;

    auto static Init() -> bool;
    void static Done();

private:
    auto codepoint_to_glyphindex(u32 cp) -> u32;

    FT_Face _face {nullptr};

    u32                                                   _fontSize {0};
    std::unordered_map<u32, u32>                          _glyphIndices {};
    std::unordered_map<u32, std::unordered_map<u32, f32>> _kerningCache;
    font::information                                     _info {};
};

}
