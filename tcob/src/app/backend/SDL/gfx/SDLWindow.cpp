// Copyright (c) 2026 Tobias Bohnen
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
#include "tcob/core/Signal.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/RenderTexture.hpp"
#include "tcob/gfx/Window.hpp"

namespace tcob::gfx {

static auto Check(string const& msg, bool c) -> bool
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

        Check("SDL_SetWindowIcon", SDL_SetWindowIcon(_handle, surface));
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
    Check("SDL_SetWindowMouseGrab", SDL_SetWindowMouseGrab(_handle, grab));
    Check("SDL_SetWindowKeyboardGrab", SDL_SetWindowKeyboardGrab(_handle, grab));
}

void sdl_window::set_size(size_i newSize)
{
    if (newSize != get_size()) {
        if (get_fullscreen()) {
            SDL_DisplayMode mode {};
            Check("SDL_GetClosestFullscreenDisplayMode",
                  SDL_GetClosestFullscreenDisplayMode(SDL_GetDisplayForWindow(_handle), newSize.Width, newSize.Height, 0.0f, true, &mode));
            Check("SDL_SetWindowFullscreenMode",
                  SDL_SetWindowFullscreenMode(_handle, &mode));
        } else {
            Check("SDL_SetWindowSize",
                  SDL_SetWindowSize(_handle, newSize.Width, newSize.Height));
        }

        SDL_SyncWindow(_handle);
        SDL_SetWindowPosition(_handle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }

    quad q {};
    geometry::set_color(q, colors::White);
    geometry::set_position(q, {0, 0, static_cast<f32>(newSize.Width), static_cast<f32>(newSize.Height)});
    geometry::set_texcoords(q, {.UVRect = render_texture::UVRect(), .Level = 0});
    init_renderer(q);

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
    auto* sev {static_cast<SDL_Event*>(xev)};
    event ev {};
    ev.WindowID = sev->window.windowID;
    ev.Data1    = sev->window.data1;
    ev.Data2    = sev->window.data2;

    switch (sev->window.type) {
    case SDL_EVENT_WINDOW_SHOWN:           emit_signal(Shown, ev); break;
    case SDL_EVENT_WINDOW_HIDDEN:          emit_signal(Hidden, ev); break;
    case SDL_EVENT_WINDOW_EXPOSED:         emit_signal(Exposed, ev); break;
    case SDL_EVENT_WINDOW_MOVED:           emit_signal(Moved, ev); break;
    case SDL_EVENT_WINDOW_MINIMIZED:       emit_signal(Minimized, ev); break;
    case SDL_EVENT_WINDOW_MAXIMIZED:       emit_signal(Maximized, ev); break;
    case SDL_EVENT_WINDOW_RESTORED:        emit_signal(Restored, ev); break;
    case SDL_EVENT_WINDOW_MOUSE_ENTER:     emit_signal(Enter, ev); break;
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:     emit_signal(Leave, ev); break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:    emit_signal(FocusGained, ev); break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:      emit_signal(FocusLost, ev); break;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: emit_signal(Close, ev); break;
    case SDL_EVENT_WINDOW_HIT_TEST:        emit_signal(HitTest, ev); break;

    case SDL_EVENT_WINDOW_RESIZED:
        set_size({ev.Data1, ev.Data2});
        emit_signal(Resized, ev);
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
