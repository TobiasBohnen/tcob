// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <vector>

#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx::text_formatter {
////////////////////////////////////////////////////////////

struct quad_definition {
    rect_f         Rect {rect_f::Zero};
    texture_region TextureRegion {};
};

struct format_token {
    std::vector<quad_definition> Quads {};
    usize                        LineIndex {0};
};

////////////////////////////////////////////////////////////

class TCOB_API result final {
public:
    std::vector<format_token> Tokens {};
    isize                     QuadCount {0};
    size_f                    UsedSize {size_f::Zero};
    font*                     Font {nullptr};

    auto get_quad(isize idx) const -> std::optional<quad_definition>;
    auto get_line_index(isize idx) const -> std::optional<usize>;
};

////////////////////////////////////////////////////////////

TCOB_API auto format(utf8_string_view text, font& font, alignment align, size_f availableSize, f32 scale, bool kerning) -> result;
TCOB_API auto measure(utf8_string_view text, font& font, f32 availableHeight, bool kerning) -> size_f;
}
