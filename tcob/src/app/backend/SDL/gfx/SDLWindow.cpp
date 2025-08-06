// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "SDLWindow.hpp"

#include <cassert>
#include <memory>
#include <utility>

#include "tcob/core/Color.hpp"
#include "tcob/core/Logger.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/RenderTexture.hpp"
#include "tcob/gfx/Window.hpp"

namespace tcob::gfx {

auto static check(string const& msg, bool c) -> bool
{
    if (!c) {
        logger::Error(msg + ": " + SDL_GetError());
    }

    return c;
}

sdl_window::sdl_window(std::unique_ptr<render_backend::window_base> win)
    : window {std::move(win)}
    , _handle {static_cast<SDL_Window*>(get_impl()->get_handle())}
{
    SystemCursorEnabled.Changed.connect([](bool value) { value ? SDL_ShowCursor() : SDL_HideCursor(); });

    SDL_StartTextInput(_handle);

    set_size(get_size());
}

void sdl_window::load_icon(path const& file)
{
    if (auto img {image::Load(file)}) {
        auto const& info {img->info()};
        auto*       surface {
            SDL_CreateSurfaceFrom(
                info.Size.Width, info.Size.Height,
                SDL_PixelFormat::SDL_PIXELFORMAT_RGBA32,
                img->ptr(),
                info.stride())};

        check("SDL_SetWindowIcon", SDL_SetWindowIcon(_handle, surface));
        SDL_DestroySurface(surface);
    }
}

auto sdl_window::has_focus() const -> bool
{
    return (SDL_GetWindowFlags(_handle) & SDL_WINDOW_MOUSE_FOCUS)
        && (SDL_GetWindowFlags(_handle) & SDL_WINDOW_INPUT_FOCUS);
}

void sdl_window::grab_input(bool grab)
{
    check("SDL_SetWindowMouseGrab", SDL_SetWindowMouseGrab(_handle, grab));
    check("SDL_SetWindowKeyboardGrab", SDL_SetWindowKeyboardGrab(_handle, grab));
}

void sdl_window::set_size(size_i newSize)
{
    if (newSize != get_size()) {
        if (get_fullscreen()) {
            SDL_DisplayMode mode {};
            check("SDL_GetClosestFullscreenDisplayMode",
                  SDL_GetClosestFullscreenDisplayMode(SDL_GetDisplayForWindow(_handle), newSize.Width, newSize.Height, 0.0f, true, &mode));
            check("SDL_SetWindowFullscreenMode",
                  SDL_SetWindowFullscreenMode(_handle, &mode));
        } else {
            check("SDL_SetWindowSize",
                  SDL_SetWindowSize(_handle, newSize.Width, newSize.Height));
        }

        SDL_SyncWindow(_handle);
        SDL_SetWindowPosition(_handle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }

    quad q {};
    geometry::set_color(q, colors::White);
    geometry::set_position(q, {0, 0, static_cast<f32>(newSize.Width), static_cast<f32>(newSize.Height)});
    geometry::set_texcoords(q, {.UVRect = render_texture::UVRect(), .Level = 0});
    renderer().set_geometry(q);

    render_target::set_size(newSize);
}

auto sdl_window::get_size() const -> size_i
{
    i32 vpWidth {}, vpHeight {};
    SDL_GetWindowSize(_handle, &vpWidth, &vpHeight);
    return {vpWidth, vpHeight};
}

void sdl_window::process_events(void* xev)
{
    auto*       sev {static_cast<SDL_Event*>(xev)};
    event const ev {.WindowID = sev->window.windowID,
                    .Data1    = sev->window.data1,
                    .Data2    = sev->window.data2};

    switch (sev->window.type) {
    case SDL_EVENT_WINDOW_SHOWN:           Shown(ev); break;
    case SDL_EVENT_WINDOW_HIDDEN:          Hidden(ev); break;
    case SDL_EVENT_WINDOW_EXPOSED:         Exposed(ev); break;
    case SDL_EVENT_WINDOW_MOVED:           Moved(ev); break;
    case SDL_EVENT_WINDOW_MINIMIZED:       Minimized(ev); break;
    case SDL_EVENT_WINDOW_MAXIMIZED:       Maximized(ev); break;
    case SDL_EVENT_WINDOW_RESTORED:        Restored(ev); break;
    case SDL_EVENT_WINDOW_MOUSE_ENTER:     Enter(ev); break;
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:     Leave(ev); break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:    FocusGained(ev); break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:      FocusLost(ev); break;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: Close(ev); break;
    case SDL_EVENT_WINDOW_HIT_TEST:        HitTest(ev); break;

    case SDL_EVENT_WINDOW_RESIZED:
        set_size({ev.Data1, ev.Data2});
        Resized(ev);
        break;

    default: break;
    }
}

auto sdl_window::get_fullscreen() const -> bool
{
    return (SDL_GetWindowFlags(_handle) & SDL_WINDOW_FULLSCREEN) != 0;
}

void sdl_window::set_fullscreen(bool value)
{
    size_i const oldSize {get_size()};
    SDL_SetWindowFullscreen(_handle, value);
    SDL_SyncWindow(_handle);

    if (!value) {
        SDL_SetWindowBordered(_handle, true);
        SDL_SetWindowPosition(_handle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    } else {
        set_size(oldSize);
    }
}

auto sdl_window::get_title() const -> string
{
    return SDL_GetWindowTitle(_handle);
}

void sdl_window::set_title(string const& value)
{
    SDL_SetWindowTitle(_handle, value.c_str());
}

}
