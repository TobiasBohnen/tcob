// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/game/ScriptApi.hpp>

#include "script-api/CanvasApi.hpp"

namespace tcob {

ScriptApi::ScriptApi(Game& game, const std::string& ns)
    : _game { game }
    , _namespace { ns }
{
}

auto ScriptApi::create_script() -> std::unique_ptr<LuaScript>
{
    auto retValue { std::make_unique<LuaScript>() };
    retValue->open_libraries();

    //.Canvas
    detail::create_canvas_wrapper(retValue.get(), _game.resources());
    auto ns { retValue->global_table().create_table(_namespace) };
    ns["Canvas"] = &_canvas;

    //.Colors
    detail::fill_colors_table(ns.create_table("Colors"));

    return retValue;
}

}
