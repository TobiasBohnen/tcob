// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/game/Game.hpp>
#include <tcob/gfx/Canvas.hpp>
#include <tcob/script/LuaScript.hpp>

namespace tcob {
class ScriptApi {
public:
    ScriptApi(Game& game, const std::string& ns);

    virtual auto create_script() -> std::unique_ptr<lua::Script>;

private:
    Game& _game;
    Canvas _canvas {};

    std::string _namespace;
};
}