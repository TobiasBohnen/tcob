// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/RenderSystem.hpp"

#include <SDL.h>

namespace tcob::gfx {

render_system::render_system() = default;

render_system::~render_system()
{
    _window        = nullptr;
    _defaultTarget = nullptr;
}

auto render_system::init_window(video_config const& config, string const& windowTitle) -> window&
{
    size_i const resolution {config.UseDesktopResolution ? get_desktop_size(0) : config.Resolution};

    _window = std::unique_ptr<window> {new window(create_window(resolution))};

    _window->FullScreen(config.FullScreen || config.UseDesktopResolution);
    _window->VSync(config.VSync);
    _window->Size(resolution);
    _window->Title = windowTitle;

    _defaultTarget = std::make_unique<gfx::default_render_target>();

    _window->clear();
    _window->draw_to(*_defaultTarget);
    _window->swap_buffer();

    return *_window;
}

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

auto render_system::get_window() const -> gfx::window&
{
    return *_window;
}

auto render_system::get_default_target() const -> gfx::default_render_target&
{
    return *_defaultTarget;
}
}
