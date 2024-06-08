// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <future>
#include <unordered_map>
#include <vector>

#include "tcob/core/FlatMap.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/TypeFactory.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/Texture.hpp"

#include "tcob/core/ext/magic_enum_reduced.hpp"

using FT_Face = struct FT_FaceRec_*;

namespace tcob::gfx {
////////////////////////////////////////////////////////////

struct glyph {
    size_i  Size {size_i::Zero};
    point_f Offset {point_f::Zero};
    f32     AdvanceX {0.0f};
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

class TCOB_API font : public non_copyable {
public:
    ////////////////////////////////////////////////////////////
    enum class weight {
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

        void static Serialize(style const& v, auto&& s);
        auto static Deserialize(style& v, auto&& s) -> bool;
    };

    struct info final {
        f32 Ascender {0};
        f32 Descender {0};
        f32 LineHeight {0};
    };
    ////////////////////////////////////////////////////////////

    font();
    virtual ~font() = default;

    auto virtual get_info() const -> info const& = 0;
    auto get_texture() const -> assets::asset_ptr<texture>;

    auto virtual render_text(utf8_string_view text, bool kerning, bool readOnlyCache) -> std::vector<rendered_glyph> = 0;

    static inline char const* asset_name {"font"};

protected:
    void virtual setup_texture() = 0;

private:
    assets::manual_asset_ptr<texture> _texture {};
};

inline auto operator==(font::style const& lhs, font::style const& rhs)
{
    return lhs.IsItalic == rhs.IsItalic && lhs.Weight == rhs.Weight;
}

inline void font::style::Serialize(style const& v, auto&& s)
{
    s["is_italic"] = v.IsItalic;
    s["weight"]    = v.Weight;
}

inline auto font::style::Deserialize(style& v, auto&& s) -> bool
{
    return s.try_get(v.IsItalic, "is_italic")
        && s.try_get(v.Weight, "weight");
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API raster_font final : public font {
public:
    ////////////////////////////////////////////////////////////

    class TCOB_API loader : public non_copyable {
    public:
        struct factory : public type_factory<std::unique_ptr<loader>> {
            static inline char const* service_name {"gfx::raster_font::factory"};
        };

        loader()          = default;
        virtual ~loader() = default;

        auto virtual load [[nodiscard]] (raster_font& font, path const& file, string const& textureFolder) -> std::optional<font::info> = 0;
    };

    ////////////////////////////////////////////////////////////

    raster_font();
    ~raster_font() override = default;

    auto get_info() const -> font::info const& override;

    auto load [[nodiscard]] (path const& file, string const& textureFolder) noexcept -> load_status;
    auto load_async [[nodiscard]] (path const& file, string const& textureFolder) noexcept -> std::future<load_status>;

    auto render_text(utf8_string_view text, bool kerning, bool readOnlyCache) -> std::vector<rendered_glyph> override;

    void add_image(image const& img);
    void add_glyph(u32 idx, rendered_glyph const& gl);
    void add_kerning_pair(u32 first, u32 second, i16 amount);

protected:
    void setup_texture() override;

private:
    font::info _info {};

    std::unordered_map<u32, rendered_glyph>               _glyphs {};
    std::unordered_map<u32, std::unordered_map<u32, i16>> _kerning {};

    std::vector<image> _fontImages {};
    bool               _textureNeedsUpdate {false};
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API truetype_font_engine : public non_copyable {
public:
    truetype_font_engine() = default;
    ~truetype_font_engine();

    auto load_data(std::span<ubyte const> data, u32 fontsize) -> std::optional<font::info>;
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
    font::info                   _info {};
};

////////////////////////////////////////////////////////////

class TCOB_API truetype_font final : public font {
public:
    truetype_font();

    auto get_info() const -> font::info const& override;

    auto load [[nodiscard]] (path const& filename, u32 size) noexcept -> load_status;
    auto load [[nodiscard]] (istream& stream, u32 size) noexcept -> load_status;
    auto load [[nodiscard]] (std::span<ubyte const> fontData, u32 size) noexcept -> load_status;

    auto render_text(utf8_string_view text, bool kerning, bool readOnlyCache) -> std::vector<rendered_glyph> override;
    void decompose_text(utf8_string_view text, bool kerning, decompose_callbacks& funcs);

protected:
    void setup_texture() override;

private:
    auto cache_render_glyph(u32 cp) -> bool;

    std::unordered_map<u32, rendered_glyph> _renderGlyphCache {};
    point_i                                 _fontTextureCursor {point_i::Zero};
    u32                                     _fontTextureLayer {0};
    bool                                    _textureNeedsSetup {false};

    info               _info;
    std::vector<ubyte> _fontData {};

    truetype_font_engine _engine;
};

////////////////////////////////////////////////////////////

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

    static inline char const* asset_name {"font_family"};

private:
    void set_source(font::style style, path const& file);

    string _name;

    flat_map<font::style, path>                                                   _fontSources;
    flat_map<font::style, std::vector<ubyte>>                                     _fontData;
    flat_map<font::style, flat_map<u32, assets::manual_asset_ptr<truetype_font>>> _fontAssets;
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
