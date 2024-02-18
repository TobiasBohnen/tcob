// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

#include <tcob/tcob.hpp>

auto main(int argc, char* argv[]) -> int
{
    tcob::game game {{.Path = argv[0],
                      .Name = "TestGame"}};
    game.push_scene<start_scene>();
    game.start();
    return 0;
}
