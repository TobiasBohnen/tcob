// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "UIEx.hpp"

#include <tcob/tcob.hpp>

auto main(int argc, char* argv[]) -> int
{
    tcob::game game {{.Path    = argv[0],
                      .Name    = "UIEx",
                      .LogFile = "stdout"}};

    auto& resMgr {game.get_library()};
    auto& resGrp {resMgr.create_or_get_group("ui")};
    resGrp.mount("./uiex.zip");
    resMgr.load_all_groups();

    game.push_scene<UIEx>();
    game.start();
    return 0;
}
