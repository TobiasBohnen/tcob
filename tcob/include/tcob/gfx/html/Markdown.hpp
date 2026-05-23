// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

namespace tcob::gfx::html {
////////////////////////////////////////////////////////////

TCOB_API auto md_to_html(utf8_string_view md) -> utf8_string;

}
