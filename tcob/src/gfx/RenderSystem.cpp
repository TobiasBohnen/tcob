// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/RenderSystem.hpp"

#include <SDL.h>

namespace tcob::gfx {

auto render_system::get_stats() -> stats&
{
    return _stats;
}

auto render_system::get_displays() const -> std::map<i32, display>
{
    std::map<i32, display> retValue;

    SDL_DisplayMode mode {};
    i32 const       numDisplays {SDL_GetNumVideoDisplays()};
    for (i32 i {0}; i < numDisplays; ++i) {
        i32 const numModes {SDL_GetNumDisplayModes(i)};
        for (i32 j {0}; j < numModes; ++j) {
            SDL_GetDisplayMode(i, j, &mode);
            retValue[i].Modes.push_back({.Size        = {mode.w, mode.h},
                                         .RefreshRate = mode.refresh_rate});
        }

        SDL_GetDesktopDisplayMode(i, &mode);
        retValue[i].DesktopMode = {.Size        = {mode.w, mode.h},
                                   .RefreshRate = mode.refresh_rate};
    }

    return retValue;
}

auto render_system::get_desktop_size(i32 display) const -> size_i
{
    SDL_DisplayMode mode;
    SDL_GetDesktopDisplayMode(display, &mode);
    return {mode.w, mode.h};
}

}
