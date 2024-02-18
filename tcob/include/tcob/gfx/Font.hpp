// Copyright (c) 2023 Tobias Bohnen
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

namespace tcob::gfx {
////////////////////////////////////////////////////////////

struct glyph {
    size_i         Size {size_i::Zero};
    point_f        Offset {point_f::Zero};
    f32            AdvanceX {0.0f};
    texture_region TexRegion {rect_f::Zero, 0};
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

    auto virtual shape_text(utf8_string_view text, bool kerning, bool readOnlyCache) -> std::vector<glyph> = 0;

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

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API raster_font final : public font {
public:
    ////////////////////////////////////////////////////////////

    class TCOB_API loader : public non_copyable {
    public:
        struct factory : public type_factory<std::unique_ptr<loader>> {
            static inline char const* service_name {"gfx::raster_font::raster_font::factory"};
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

    auto shape_text(utf8_string_view text, bool kerning, bool readOnlyCache) -> std::vector<glyph> override;

    void add_image(image const& img);
    void add_glyph(u32 idx, glyph const& gl);
    void add_kerning_pair(u32 first, u32 second, i16 amount);

protected:
    void setup_texture() override;

private:
    font::info _info {};

    std::unordered_map<u32, glyph>                        _glyphs {};
    std::unordered_map<u32, std::unordered_map<u32, i16>> _kerning {};

    std::vector<image> _fontImages {};
    bool               _textureNeedsUpdate {false};
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API truetype_font_engine : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<truetype_font_engine>> {
        static inline char const* service_name {"gfx::truetype_font_engine::factory"};
    };

    struct glyph_bitmap {
        glyph              Glyph {};
        std::vector<ubyte> Bitmap {};
    };

    truetype_font_engine()          = default;
    virtual ~truetype_font_engine() = default;

    auto virtual get_name() -> string = 0;

    auto virtual load_data(std::span<ubyte const> data, u32 fontsize) -> std::optional<font::info> = 0;

    auto virtual get_kerning(u32 cp0, u32 cp1) -> f32 = 0;
    auto virtual get_glyph(u32 cp) -> glyph_bitmap    = 0;

    static string Engine;
};

////////////////////////////////////////////////////////////

class TCOB_API truetype_font final : public font {
public:
    truetype_font();

    auto get_info() const -> font::info const& override;

    auto load [[nodiscard]] (path const& filename, u32 size) noexcept -> load_status;
    auto load [[nodiscard]] (istream& stream, u32 size) noexcept -> load_status;
    auto load [[nodiscard]] (std::span<ubyte const> fontData, u32 size) noexcept -> load_status;

    auto shape_text(utf8_string_view text, bool kerning, bool readOnlyCache) -> std::vector<glyph> override;

protected:
    void setup_texture() override;

private:
    auto cache_glyph(u32 cp) -> bool;

    std::unordered_map<u32, glyph> _glyphs {};
    point_i                        _fontTextureCursor {point_i::Zero};
    u32                            _fontTextureLayer {0};
    info                           _info;
    bool                           _textureNeedsSetup {false};
    std::vector<ubyte>             _fontData {};

    std::unique_ptr<truetype_font_engine> _engine;
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
