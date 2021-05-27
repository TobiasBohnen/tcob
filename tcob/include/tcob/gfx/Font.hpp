// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <unordered_map>

#include <tcob/assets/Resource.hpp>
#include <tcob/core/data/Rect.hpp>
#include <tcob/gfx/Material.hpp>
#include <tcob/gfx/gl/GLTexture.hpp>

struct stbtt_fontinfo;

namespace tcob {
struct Glyph {
    PointF Bearing { 0.f, 0.f };
    SizeF Size { 0.f, 0.f };
    f32 Offset { 0.f };
    f32 Advance { 0.f };
    RectF UVRect { 0.f, 0.f, 0.f, 0.f };
};

struct FontInfo final {
    f32 Ascender { 0 };
    f32 Descender { 0 };
    f32 Height { 0 };
};

class Font final {
public:
    Font();
    ~Font();
    Font(const Font&) = delete;
    auto operator=(const Font& other) -> Font& = delete;

    auto load(const std::string& filename, u32 fontSize) -> bool;

    auto material() const -> ResourcePtr<Material>;
    void material(ResourcePtr<Material> material);

    auto texture() const -> ResourcePtr<gl::TextureBase>;

    auto info() const -> FontInfo;

    void kerning(bool kerning);

    auto shape_text(const std::string& text) -> std::vector<Glyph>;

    static inline ResourcePtr<Font> Default;
    static inline ResourcePtr<gl::ShaderProgram> DefaultShader;

private:
    auto ascender() const -> f32;
    auto descender() const -> f32;
    auto height() const -> f32;

    void create_texture();

    auto cache_glyph(u32 codepoint) -> bool;
    auto glyph_index(u32 codepoint) -> u32;

    stbtt_fontinfo* _fontInfo;
    std::vector<ubyte> _fontData;

    f32 _fontScale { 0 };
    u32 _fontSize { 0 };
    i32 _ascent { 0 };
    i32 _descent { 0 };
    i32 _lineGap { 0 };

    std::unordered_map<u32, Glyph> _glyphs;
    std::unordered_map<u32, u32> _glyphIndices;
    PointU _fontTextureCursor { PointU::Zero };

    std::shared_ptr<gl::Texture2D> _fontTexture;
    std::shared_ptr<Material> _material;
    ResourcePtr<Material> _matRes;

    bool _kerning { true };
};
}