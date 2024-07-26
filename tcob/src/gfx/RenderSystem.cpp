// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/RenderSystem.hpp"

namespace tcob::gfx {

auto render_system::get_stats() -> stats&
{
    return _stats;
}

}
