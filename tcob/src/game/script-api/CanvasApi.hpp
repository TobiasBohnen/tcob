// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/assets/ResourceLibrary.hpp>
#include <tcob/gfx/Canvas.hpp>
#include <tcob/script/LuaScript.hpp>

namespace tcob::detail {

void create_canvas_wrapper(lua::Script* script, const ResourceLibrary& library);
void fill_colors_table(const lua::Table& targetTable);

}