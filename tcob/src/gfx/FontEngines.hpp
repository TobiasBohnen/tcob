// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <unordered_map>

#include "tcob/gfx/Font.hpp"

////////////////////////////////////////////////////////////

#if defined(TCOB_ENABLE_TTF_FREETYPE)

    #include <ft2build.h>
    #include FT_FREETYPE_H

namespace tcob::gfx::detail {

class ft_ttf_font_engine : public truetype_font_engine {
public:
    ft_ttf_font_engine() = default;
    ~ft_ttf_font_engine() override;

    auto get_name() -> string override;

    auto load_data(std::span<ubyte const> data, u32 fontsize) -> std::optional<font::info> override;
    auto get_kerning(u32 cp0, u32 cp1) -> f32 override;
    auto get_glyph(u32 cp) -> glyph_bitmap override;

    auto static Init() -> bool;
    void static Done();

private:
    auto codepoint_to_glyphindex(u32 cp) -> u32;

    FT_Face _face {nullptr};

    u32                          _fontSize {0};
    std::unordered_map<u32, u32> _glyphIndices {};
    font::info                   _info {};
};
}

#endif

////////////////////////////////////////////////////////////

#if defined(TCOB_ENABLE_TTF_STBTT)

struct stbtt_fontinfo;

namespace tcob::gfx::detail {

class stb_ttf_font_engine : public truetype_font_engine {
public:
    stb_ttf_font_engine();
    ~stb_ttf_font_engine() override;

    auto get_name() -> string override;

    auto load_data(std::span<ubyte const> data, u32 fontsize) -> std::optional<font::info> override;
    auto get_kerning(u32 cp0, u32 cp1) -> f32 override;
    auto get_glyph(u32 cp) -> glyph_bitmap override;

private:
    auto codepoint_to_glyphindex(u32 cp) -> u32;

    stbtt_fontinfo*              _handle {nullptr};
    f32                          _fontScale {0};
    std::unordered_map<u32, u32> _glyphIndices {};
    font::info                   _info {};
};
}

#endif

////////////////////////////////////////////////////////////

#if defined(TCOB_ENABLE_TTF_LIBSCHRIFT)

struct SFT;

namespace tcob::gfx::detail {

class libschrift_ttf_font_engine : public truetype_font_engine {
public:
    libschrift_ttf_font_engine();
    ~libschrift_ttf_font_engine() override;

    auto get_name() -> string override;

    auto load_data(std::span<ubyte const> data, u32 fontsize) -> std::optional<font::info> override;
    auto get_kerning(u32 cp0, u32 cp1) -> f32 override;
    auto get_glyph(u32 cp) -> glyph_bitmap override;

private:
    auto codepoint_to_glyphindex(u32 cp) -> u32;

    SFT*                         _handle {nullptr};
    std::unordered_map<u32, u32> _glyphIndices {};
    font::info                   _info {};
};
}

#endif
