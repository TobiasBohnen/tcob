// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <unordered_map>
#include <vector>

#include <tcob/assets/Resource.hpp>
#include <tcob/core/data/Rect.hpp>
#include <tcob/gfx/Material.hpp>
#include <tcob/gfx/gl/GLTexture.hpp>
#include <tcob/thirdparty/sigslot/signal.hpp>

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
    f32 Linegap { 0 };
    f32 Height { 0 };
};

////////////////////////////////////////////////////////////

class Font {
public:
    Font();
    ~Font();

    sigslot::signal<> Changed;

    virtual auto load(const std::string& filename, u32 fontSize) -> bool = 0;

    auto material() const -> ResourcePtr<Material>;
    void material(ResourcePtr<Material> material);
    auto texture() const -> gl::Texture2D*;

    auto info() const -> FontInfo;

    auto kerning() const -> bool;
    void kerning(bool kerning);

    void line_gap_override(f32 val);

    virtual auto shape_text(const std::string& text) -> std::vector<Glyph> = 0;

    static inline ResourcePtr<Font> Default;
    static inline ResourcePtr<gl::ShaderProgram> DefaultShader;

protected:
    virtual auto ascender() const -> f32 = 0;
    virtual auto descender() const -> f32 = 0;
    virtual auto linegap() const -> f32 = 0;

private:
    std::shared_ptr<gl::Texture2D> _fontTexture;
    std::shared_ptr<Material> _material;
    ResourcePtr<Material> _matRes;

    std::optional<f32> _lineGapOverride;

    bool _kerning { true };
};

////////////////////////////////////////////////////////////

class TrueTypeFont final : public Font {
public:
    explicit TrueTypeFont(bool sdfmode);
    ~TrueTypeFont();
    TrueTypeFont(const TrueTypeFont&) = delete;
    auto operator=(const TrueTypeFont& other) -> TrueTypeFont& = delete;

    auto load(const std::string& filename, u32 fontSize) -> bool override;

    auto shape_text(const std::string& text) -> std::vector<Glyph> override;

protected:
    auto ascender() const -> f32 override;
    auto descender() const -> f32 override;
    auto linegap() const -> f32 override;

private:
    void create_texture();
    auto cache_glyph(u32 codepoint) -> bool;
    auto codepoint_to_glyphindex(u32 codepoint) -> u32;

    stbtt_fontinfo* _fontInfo;
    std::vector<ubyte> _fontData;

    f32 _fontScale { 0 };
    u32 _fontSize { 0 };
    i32 _ascent { 0 };
    i32 _descent { 0 };
    i32 _lineGap { 0 };

    bool _sdfMode { false };

    std::unordered_map<u32, Glyph> _glyphs;
    std::unordered_map<u32, u32> _glyphIndices;
    PointU _fontTextureCursor { PointU::Zero };
};
}