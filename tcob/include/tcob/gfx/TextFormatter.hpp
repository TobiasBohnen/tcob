// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <variant>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx::text_formatter {

////////////////////////////////////////////////////////////

enum class token_type : u8 {
    None,
    Text,
    Whitespace,
    Newline,
    Command
};

enum class command_type : u8 {
    None,
    Color,
    Alpha,
    Effect
};

////////////////////////////////////////////////////////////

struct command_definition {
    command_type            Type {command_type::None};
    std::variant<color, u8> Value;
};

struct quad_definition {
    rect_f         Rect {rect_f::Zero};
    texture_region TexRegion {};
};

struct token {
    token_type         Type {token_type::None}; // shape
    string             Text {};                 // shape
    command_definition Command {};              // shape
    f32                Width {0};               // shape
    std::vector<glyph> Glyphs {};               // shape
};

struct line_definition {
    std::vector<token const*> Tokens {};
    f32                       RemainingWidth {0};
    f32                       WhiteSpaceCount {0};
};

struct format_token {
    command_definition           Command {}; // shape
    std::vector<quad_definition> Quads {};   // format
};

////////////////////////////////////////////////////////////

class TCOB_API result {
public:
    std::vector<format_token> Tokens {};
    usize                     QuadCount {0};
    size_f                    UsedSize {size_f::Zero};
    font*                     Font {nullptr};

    auto get_quad(usize idx) -> quad_definition;
};

////////////////////////////////////////////////////////////

TCOB_API auto shape(utf8_string_view text, font& font, bool kerning, bool readOnlyCache) -> std::vector<token>;
TCOB_API auto wrap(std::vector<token> const& tokens, f32 lineWidth, f32 scale) -> std::vector<line_definition>;
TCOB_API auto format(std::vector<line_definition> const& lines, font& font, alignments align, f32 availableHeight, f32 scale) -> result;

TCOB_API auto format_text(utf8_string_view text, font& font, alignments align, size_f availableSize, f32 scale, bool kerning) -> result;
TCOB_API auto measure_text(utf8_string_view text, font& font, f32 availableHeight, bool kerning) -> size_f;
}
