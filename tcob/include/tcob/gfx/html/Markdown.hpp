// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

namespace tcob::gfx::html {
////////////////////////////////////////////////////////////

namespace detail {
    class TCOB_API md {
    public:
        std::vector<utf8_string> Lines;
        usize                    CurrentPos {0};

        auto parse() -> utf8_string;

    private:
        auto is_eof() const -> bool;

        auto block() -> utf8_string;

        auto fenced_code() -> utf8_string;

        auto indented_code() -> utf8_string;

        auto atx_heading(i32 level) -> utf8_string;

        auto blockquote() -> utf8_string;

        auto list(bool ordered) -> utf8_string;

        auto table() -> utf8_string;

        auto paragraph() -> utf8_string;
    };

}

////////////////////////////////////////////////////////////

TCOB_API auto md_to_html(utf8_string_view md) -> utf8_string;

}
