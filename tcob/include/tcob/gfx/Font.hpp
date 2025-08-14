// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <cstddef>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Texture.hpp"

#include "tcob/core/ext/magic_enum_reduced.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API glyph {
public:
    size_i  Size {size_i::Zero};
    point_f Offset {point_f::Zero};
    f32     AdvanceX {0.0f};

    std::optional<texture_region> TextureRegion;

    auto operator==(glyph const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

struct decompose_callbacks {
    std::function<void(point_f)>                   MoveTo;
    std::function<void(point_f)>                   LineTo;
    std::function<void(point_f, point_f)>          ConicTo;
    std::function<void(point_f, point_f, point_f)> CubicTo;
    point_f                                        Offset {};
};

struct decompose_move {
    point_f Point;
};
struct decompose_line {
    point_f Point;
};
struct decompose_conic {
    point_f Point0;
    point_f Point1;
};
struct decompose_cubic {
    point_f Point0;
    point_f Point1;
    point_f Point2;
};

using decompose_commands = std::variant<decompose_move, decompose_line, decompose_conic, decompose_cubic>;

struct decompose_result {
    u32                             CodePoint;
    std::vector<decompose_commands> Commands;
};

////////////////////////////////////////////////////////////
class truetype_font_engine;

class TCOB_API font : public non_copyable {
public:
    ////////////////////////////////////////////////////////////
    struct information final {
        f32 Ascender {0};
        f32 Descender {0};
        f32 LineHeight {0};

        auto operator==(information const& other) const -> bool = default;
    };

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

        auto constexpr static Members()
        {
            return std::tuple {
                std::pair {"is_italic", &font::style::IsItalic},
                std::pair {"weight", &font::style::Weight}};
        }
    };

    ////////////////////////////////////////////////////////////

    font();
    virtual ~font();

    auto info() const -> information const&;
    auto texture() const -> assets::asset_ptr<gfx::texture>;

    auto load [[nodiscard]] (path const& filename, u32 size) noexcept -> bool;
    auto load [[nodiscard]] (io::istream& stream, u32 size) noexcept -> bool;
    auto load [[nodiscard]] (std::span<byte const> fontData, u32 size) noexcept -> bool;

    auto render_text(utf8_string_view text, bool kerning) -> std::vector<glyph>;
    void decompose_text(utf8_string_view text, bool kerning, decompose_callbacks& funcs);

    auto get_glyphs(utf8_string_view text, bool kerning) -> std::vector<glyph>;

    static inline char const* AssetName {"font"};

    auto static Init() -> bool;
    void static Done();

private:
    void setup_texture();

    auto cache_render_glyph(u32 cp) -> bool;

    std::unordered_map<u32, glyph>            _glyphCache {};
    std::unordered_map<u32, decompose_result> _decomposeCache {};

    point_i _fontTextureCursor {point_i::Zero};
    u32     _fontTextureLayer {0};
    bool    _textureNeedsSetup {false};

    information       _info;
    std::vector<byte> _fontData {};

    std::unique_ptr<truetype_font_engine> _engine;

    assets::asset_owner_ptr<gfx::texture> _texture {};
};

////////////////////////////////////////////////////////////
}

template <>
struct tcob::detail::magic_enum_reduced::custom_range<tcob::gfx::font::weight> {
    static constexpr i32 Min {100};
    static constexpr i32 Max {900};
};

template <>
struct std::hash<tcob::gfx::font::style> {
    auto operator()(tcob::gfx::font::style const& s) const noexcept -> std::size_t
    {
        std::size_t const h1 {std::hash<bool> {}(s.IsItalic)};
        std::size_t const h2 {std::hash<int> {}(static_cast<int>(s.Weight))};
        return tcob::helper::hash_combine(h1, h2);
    }
};

template <>
struct std::formatter<tcob::gfx::font::style> {
    auto constexpr parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(tcob::gfx::font::style val, format_context& ctx) const
    {
        string weight;
        switch (val.Weight) {
        case tcob::gfx::font::weight::Thin:       weight = "Thin"; break;
        case tcob::gfx::font::weight::ExtraLight: weight = "ExtraLight"; break;
        case tcob::gfx::font::weight::Light:      weight = "Light"; break;
        case tcob::gfx::font::weight::Normal:     weight = "Normal"; break;
        case tcob::gfx::font::weight::Medium:     weight = "Medium"; break;
        case tcob::gfx::font::weight::SemiBold:   weight = "SemiBold"; break;
        case tcob::gfx::font::weight::Bold:       weight = "Bold"; break;
        case tcob::gfx::font::weight::ExtraBold:  weight = "ExtraBold"; break;
        case tcob::gfx::font::weight::Heavy:      weight = "Heavy"; break;
        }
        return format_to(ctx.out(), "(IsItalic:{},Weight:{})", val.IsItalic, weight);
    }
};
