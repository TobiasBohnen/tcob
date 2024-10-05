// Copyright (c) 2024 Tobias Bohnen
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

TCOB_API auto format(utf8_string_view text, font& font, alignments align, size_f availableSize, f32 scale, bool kerning) -> result;
TCOB_API auto measure(utf8_string_view text, font& font, f32 availableHeight, bool kerning) -> size_f;
}
