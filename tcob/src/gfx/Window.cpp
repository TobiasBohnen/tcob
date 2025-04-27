// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Window.hpp"

#include <cassert>
#include <memory>
#include <utility>

#include <SDL3/SDL.h>

#include "tcob/core/Color.hpp"
#include "tcob/core/Logger.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/RenderTexture.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx {

auto static check(string const& msg, bool c) -> bool
{
    if (!c) {
        logger::Error(msg + ": " + SDL_GetError());
    }

    return c;
}

window::window(std::unique_ptr<render_backend::window_base> window, assets::owning_asset_ptr<texture> const& texture)
    : render_target {texture.ptr()}
    , FullScreen {{[this]() { return get_fullscreen(); },
                   [this](auto const& value) { set_fullscreen(value); }}}
    , Title {{[this]() { return get_title(); },
              [this](auto const& value) { set_title(value); }}}
    , VSync {{[this]() { return _impl->get_vsync(); },
              [this](auto const& value) { _impl->set_vsync(value); }}}
    , _texture {texture}
    , _impl {std::move(window)}
{
    Cursor.Changed.connect([this](auto const& value) { SystemCursorEnabled = !value.is_ready(); });
    SystemCursorEnabled(true);
    SystemCursorEnabled.Changed.connect([](bool value) { value ? SDL_ShowCursor() : SDL_HideCursor(); });

    Shader.Changed.connect([this](auto const& value) { _material->Shader = value; });

    _material->Texture = _texture;
    _renderer.set_material(_material.ptr());

    set_size(Size());

    SDL_StartTextInput(_impl->get_handle());
}

window::~window() = default;

auto window::bounds() const -> rect_i
{
    return {point_i::Zero, Size()};
}

void window::load_icon(path const& file)
{
    if (auto img {image::Load(file)}) {
        auto const& info {img->info()};
        auto*       surface {
            SDL_CreateSurfaceFrom(
                info.Size.Width, info.Size.Height,
                SDL_PixelFormat::SDL_PIXELFORMAT_RGBA32,
                img->ptr(),
                info.stride())};

        check("SDL_SetWindowIcon", SDL_SetWindowIcon(_impl->get_handle(), surface));
        SDL_DestroySurface(surface);
    }
}

auto window::has_focus() const -> bool
{
    return (SDL_GetWindowFlags(_impl->get_handle()) & SDL_WINDOW_MOUSE_FOCUS)
        && (SDL_GetWindowFlags(_impl->get_handle()) & SDL_WINDOW_INPUT_FOCUS);
}

void window::grab_input(bool grab)
{
    check("SDL_SetWindowMouseGrab", SDL_SetWindowMouseGrab(_impl->get_handle(), grab));
    check("SDL_SetWindowKeyboardGrab", SDL_SetWindowKeyboardGrab(_impl->get_handle(), grab));
}

void window::on_clear(color c) const
{
    _impl->clear(c);
}

void window::set_size(size_i newSize)
{
    auto* win {_impl->get_handle()};
    if (newSize != get_size()) {
        if (get_fullscreen()) {
            SDL_DisplayMode mode {};
            check("SDL_GetClosestFullscreenDisplayMode",
                  SDL_GetClosestFullscreenDisplayMode(SDL_GetDisplayForWindow(win), newSize.Width, newSize.Height, 0.0f, true, &mode));
            check("SDL_SetWindowFullscreenMode", SDL_SetWindowFullscreenMode(win, &mode));
        } else {
            check("SDL_SetWindowSize",
                  SDL_SetWindowSize(_impl->get_handle(), newSize.Width, newSize.Height));
        }

        SDL_SyncWindow(win);
        SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }

    quad q {};
    geometry::set_color(q, colors::White);
    geometry::set_position(q, {0, 0, static_cast<f32>(newSize.Width), static_cast<f32>(newSize.Height)});
    geometry::set_texcoords(q, {.UVRect = render_texture::GetTexcoords(), .Level = 0});
    _renderer.set_geometry(q);

    render_target::set_size(newSize);
}

auto window::get_size() const -> size_i
{
    i32 vpWidth {}, vpHeight {};
    SDL_GetWindowSize(_impl->get_handle(), &vpWidth, &vpHeight);
    return {vpWidth, vpHeight};
}

void window::process_events(SDL_Event* sdlEv)
{
    event const ev {.WindowID = sdlEv->window.windowID,
                    .Data1    = sdlEv->window.data1,
                    .Data2    = sdlEv->window.data2};

    switch (sdlEv->window.type) {
    case SDL_EVENT_WINDOW_SHOWN:   Shown(ev); break;
    case SDL_EVENT_WINDOW_HIDDEN:  Hidden(ev); break;
    case SDL_EVENT_WINDOW_EXPOSED: Exposed(ev); break;
    case SDL_EVENT_WINDOW_MOVED:   Moved(ev); break;
    case SDL_EVENT_WINDOW_RESIZED:
        set_size({ev.Data1, ev.Data2});
        Resized(ev);
        break;
    case SDL_EVENT_WINDOW_MINIMIZED:       Minimized(ev); break;
    case SDL_EVENT_WINDOW_MAXIMIZED:       Maximized(ev); break;
    case SDL_EVENT_WINDOW_RESTORED:        Restored(ev); break;
    case SDL_EVENT_WINDOW_MOUSE_ENTER:     Enter(ev); break;
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:     Leave(ev); break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:    FocusGained(ev); break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:      FocusLost(ev); break;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: Close(ev); break;
    case SDL_EVENT_WINDOW_HIT_TEST:        HitTest(ev); break;
    default:                               break;
    }
}

void window::draw_to(render_target& target)
{
    _impl->set_viewport({point_i::Zero, get_size()});
    _renderer.render_to_target(target);

    if (Cursor()) {
        Cursor->draw_to(target);
    }
}

void window::swap_buffer() const
{
    _impl->swap_buffer();
}

auto window::get_fullscreen() const -> bool
{
    return (SDL_GetWindowFlags(_impl->get_handle()) & SDL_WINDOW_FULLSCREEN) != 0;
}

void window::set_fullscreen(bool value)
{
    auto*        win {_impl->get_handle()};
    size_i const oldSize {get_size()};
    SDL_SetWindowFullscreen(win, value);
    SDL_SyncWindow(win);

    if (!value) {
        SDL_SetWindowBordered(win, true);
        SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    } else {
        set_size(oldSize);
    }
}

auto window::get_title() const -> string
{
    return SDL_GetWindowTitle(_impl->get_handle());
}

void window::set_title(string const& value)
{
    SDL_SetWindowTitle(_impl->get_handle(), value.c_str());
}
}
