// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <unordered_map>
#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/Polygon.hpp"
#include "tcob/gfx/Texture.hpp"

#include "tcob/core/ext/magic_enum_reduced.hpp"

using FT_Face = struct FT_FaceRec_*;

namespace tcob::gfx {
////////////////////////////////////////////////////////////

struct font_info final {
    f32 Ascender {0};
    f32 Descender {0};
    f32 LineHeight {0};

    auto operator==(font_info const& other) const -> bool = default;
};

struct glyph {
    size_i  Size {size_i::Zero};
    point_f Offset {point_f::Zero};
    f32     AdvanceX {0.0f};

    auto operator==(glyph const& other) const -> bool = default;
};

struct rendered_glyph : glyph {
    texture_region TexRegion {rect_f::Zero, 0};
};

struct glyph_bitmap {
    std::vector<ubyte> Bitmap {};
    size_i             BitmapSize {};
};

struct decompose_callbacks {
    std::function<void(point_f)>                   MoveTo;
    std::function<void(point_f)>                   LineTo;
    std::function<void(point_f, point_f)>          ConicTo;
    std::function<void(point_f, point_f, point_f)> CubicTo;
    point_f                                        Offset {};
};

////////////////////////////////////////////////////////////

class TCOB_API truetype_font_engine : public non_copyable {
public:
    truetype_font_engine() = default;
    ~truetype_font_engine();

    auto load_data(std::span<ubyte const> data, u32 fontsize) -> std::optional<font_info>;
    auto get_kerning(u32 cp0, u32 cp1) -> f32;

    auto render_glyph(u32 cp) -> std::pair<glyph, glyph_bitmap>;
    auto decompose_glyph(u32 cp, decompose_callbacks& funcs) -> glyph;
    auto load_glyph(u32 cp) -> glyph;

    auto static Init() -> bool;
    void static Done();

private:
    auto codepoint_to_glyphindex(u32 cp) -> u32;

    FT_Face _face {nullptr};

    u32                          _fontSize {0};
    std::unordered_map<u32, u32> _glyphIndices {};
    font_info                    _info {};
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API font : public non_copyable {
public:
    ////////////////////////////////////////////////////////////
    enum class weight : u16 {
        Thin       = 100, // Hairline
        ExtraLight = 200, // Ultra Light
        Light      = 300,
        Normal     = 400, // Regular
        Medium     = 500,
        SemiBold   = 600, // Demi Bold
        Bold       = 700,
        ExtraBold  = 800, // Ultra Bold
        Heavy      = 900  // Black
    };

    struct style {
        bool   IsItalic {false};
        weight Weight {weight::Normal};

        auto operator==(font::style const& other) const -> bool = default;
    };

    void Serialize(font::style const& v, auto&& s)
    {
        s["is_italic"] = v.IsItalic;
        s["weight"]    = v.Weight;
    }

    auto Deserialize(font::style& v, auto&& s) -> bool
    {
        return s.try_get(v.IsItalic, "is_italic")
            && s.try_get(v.Weight, "weight");
    }

    ////////////////////////////////////////////////////////////

    font();
    virtual ~font() = default;

    signal<std::span<ubyte>> Render;

    auto get_info() const -> font_info const&;
    auto get_texture() const -> assets::asset_ptr<texture>;

    auto load [[nodiscard]] (path const& filename, u32 size) noexcept -> load_status;
    auto load [[nodiscard]] (io::istream& stream, u32 size) noexcept -> load_status;
    auto load [[nodiscard]] (std::span<ubyte const> fontData, u32 size) noexcept -> load_status;

    auto render_text(utf8_string_view text, bool kerning, bool readOnlyCache) -> std::vector<rendered_glyph>;
    auto polygonize_text(utf8_string_view text, bool kerning) -> std::vector<polygon>;
    void decompose_text(utf8_string_view text, bool kerning, decompose_callbacks& funcs);

    static inline char const* asset_name {"font"};

private:
    void setup_texture();

    auto cache_render_glyph(u32 cp) -> bool;

    std::unordered_map<u32, rendered_glyph> _renderGlyphCache {};
    point_i                                 _fontTextureCursor {point_i::Zero};
    u32                                     _fontTextureLayer {0};
    bool                                    _textureNeedsSetup {false};

    font_info          _info;
    std::vector<ubyte> _fontData {};

    truetype_font_engine _engine;

    assets::manual_asset_ptr<texture> _texture {};
};

////////////////////////////////////////////////////////////
}

namespace std {
template <>
struct hash<tcob::gfx::font::style> {
    auto operator()(tcob::gfx::font::style const& s) const noexcept -> std::size_t
    {
        std::size_t hash_value = 0;
        hash_value ^= std::hash<bool> {}(s.IsItalic) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
        hash_value ^= std::hash<int> {}(static_cast<int>(s.Weight)) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
        return hash_value;
    }
};
}

////////////////////////////////////////////////////////////

namespace tcob::gfx {

class TCOB_API font_family final {
public:
    explicit font_family(string name);

    auto get_name() const -> string const&;
    auto get_font(font::style style, u32 size) -> assets::asset_ptr<font>;

    auto get_fallback_style(font::style style) const -> std::optional<font::style>;
    auto has_style(font::style style) const -> bool;

    void clear_assets();

    auto get_font_count() const -> isize;

    auto get_images() const -> std::vector<image>;

    void static FindSources(font_family& fam, path const& source);
    void static SingleFont(font_family& fam, std::span<ubyte const> font);

    static inline char const* asset_name {"font_family"};

private:
    void set_source(font::style style, path const& file);

    string _name;

    std::unordered_map<font::style, path>                                                    _fontSources;
    std::unordered_map<font::style, std::vector<ubyte>>                                      _fontData;
    std::unordered_map<font::style, std::unordered_map<u32, assets::manual_asset_ptr<font>>> _fontAssets;
};

////////////////////////////////////////////////////////////
}

template <>
struct tcob::detail::magic_enum_reduced::custom_range<tcob::gfx::font::weight> {
    static constexpr i32 Min {100};
    static constexpr i32 Max {900};
};

template <>
struct std::formatter<tcob::gfx::font::style> {
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(tcob::gfx::font::style val, format_context& ctx) const
    {
        std::string weight;
        switch (val.Weight) {
        case tcob::gfx::font::weight::Thin:
            weight = "Thin";
            break;
        case tcob::gfx::font::weight::ExtraLight:
            weight = "ExtraLight";
            break;
        case tcob::gfx::font::weight::Light:
            weight = "Light";
            break;
        case tcob::gfx::font::weight::Normal:
            weight = "Normal";
            break;
        case tcob::gfx::font::weight::Medium:
            weight = "Medium";
            break;
        case tcob::gfx::font::weight::SemiBold:
            weight = "SemiBold";
            break;
        case tcob::gfx::font::weight::Bold:
            weight = "Bold";
            break;
        case tcob::gfx::font::weight::ExtraBold:
            weight = "ExtraBold";
            break;
        case tcob::gfx::font::weight::Heavy:
            weight = "Heavy";
            break;
        }
        return format_to(ctx.out(), "(IsItalic:{},Weight:{})", val.IsItalic, weight);
    }
};
