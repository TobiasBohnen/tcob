// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx::gles30 {

auto convert_enum(buffer_usage_hint usage) -> u32;
auto convert_enum(blend_func blendfunc) -> u32;
auto convert_enum(blend_equation equ) -> u32;
auto convert_enum(primitive_type type) -> u32;

}
