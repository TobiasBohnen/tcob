// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/game/ScriptApi.hpp>

#include "script-api/CanvasApi.hpp"

namespace tcob::lua {

ScriptApi::ScriptApi(Game& game, const std::string& ns)
    : _game { game }
    , _namespace { ns }
{
}

auto ScriptApi::create_script() -> std::unique_ptr<lua::Script>
{
    auto retValue { std::make_unique<lua::Script>() };
    retValue->open_libraries();

    //.Canvas
    tcob::detail::create_canvas_wrapper(retValue.get(), _game.resources());
    auto ns { retValue->global_table().create_table(_namespace) };
    ns["Canvas"] = &_canvas;

    //.Colors
    tcob::detail::fill_colors_table(ns.create_table("Colors"));

    return retValue;
}

}
