// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Window.hpp"

#include <SDL.h>

#include "tcob/gfx/RenderSystemImpl.hpp"
#include "tcob/gfx/RenderTexture.hpp"

namespace tcob::gfx {

window::window(std::unique_ptr<render_backend::window_base> window, assets::manual_asset_ptr<texture> const& texture)
    : render_target {texture.get_obj()}
    , FullScreen {{[&]() { return get_fullscreen(); },
                   [&](auto const& value) { set_fullscreen(value); }}}
    , Title {{[&]() { return get_title(); },
              [&](auto const& value) { set_title(value); }}}
    , VSync {{[&]() { return _impl->get_vsync(); },
              [&](auto const& value) { _impl->set_vsync(value); }}}
    , _texture {texture}
    , _impl {std::move(window)}
    , _window {_impl->get_handle()}
{
    FullScreen(false);
    VSync(false);
    Title("");

    Cursor.Changed.connect([&](auto const& value) { hide_system_cursor(value.is_ready()); });

    _material->Texture = _texture;
    _renderer.set_material(_material);

    set_size(Size());
}

window::~window() = default;

void window::load_icon(path const& file)
{
    if (auto img {image::Load(file)}) {
        auto const& info {img->get_info()};
        auto*       surface {
            SDL_CreateRGBSurfaceFrom(
                img->get_data().data(),
                info.Size.Width, info.Size.Height, 32, info.stride(),
                0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000)};

        SDL_SetWindowIcon(_window, surface);
        SDL_FreeSurface(surface);
    }
}

auto window::has_focus() const -> bool
{
    return (SDL_GetWindowFlags(_window) & SDL_WINDOW_MOUSE_FOCUS)
        && (SDL_GetWindowFlags(_window) & SDL_WINDOW_INPUT_FOCUS);
}

void window::grab_input(bool grab)
{
    SDL_SetWindowGrab(_window, grab ? SDL_TRUE : SDL_FALSE);
}

void window::on_clear(color c) const
{
    _impl->clear(c);
}

void window::set_size(size_i newSize)
{
    SDL_SetWindowSize(_window, newSize.Width, newSize.Height);

    quad q {};
    geometry::set_color(q, colors::White);
    geometry::set_position(q, {0, 0, static_cast<f32>(newSize.Width), static_cast<f32>(newSize.Height)});
    geometry::set_texcoords(q, {render_texture::GetTexcoords(), 0});
    _renderer.set_geometry(q);

    render_target::set_size(newSize);
}

auto window::get_size() const -> size_i
{
    i32 vpWidth {}, vpHeight {};
    SDL_GetWindowSize(_window, &vpWidth, &vpHeight);
    return {vpWidth, vpHeight};
}

void window::process_events(SDL_Event* sdlEv)
{
    event const ev {
        .WindowID = sdlEv->window.windowID,
        .Data1    = sdlEv->window.data1,
        .Data2    = sdlEv->window.data2};

    switch (sdlEv->window.event) {
    case SDL_WINDOWEVENT_SHOWN:
        WindowShown(ev);
        break;
    case SDL_WINDOWEVENT_HIDDEN:
        WindowHidden(ev);
        break;
    case SDL_WINDOWEVENT_EXPOSED:
        WindowExposed(ev);
        break;
    case SDL_WINDOWEVENT_MOVED:
        WindowMoved(ev);
        break;
    case SDL_WINDOWEVENT_RESIZED:
        WindowResized(ev);
        break;
    case SDL_WINDOWEVENT_SIZE_CHANGED: {
        size_i const newSize {ev.Data1, ev.Data2};
        if (newSize != get_size()) {
            set_size(newSize);
            WindowSizeChanged(ev);
        }
    } break;
    case SDL_WINDOWEVENT_MINIMIZED:
        WindowMinimized(ev);
        break;
    case SDL_WINDOWEVENT_MAXIMIZED:
        WindowMaximized(ev);
        break;
    case SDL_WINDOWEVENT_RESTORED:
        WindowRestored(ev);
        break;
    case SDL_WINDOWEVENT_ENTER:
        WindowEnter(ev);
        break;
    case SDL_WINDOWEVENT_LEAVE:
        WindowLeave(ev);
        break;
    case SDL_WINDOWEVENT_FOCUS_GAINED:
        WindowFocusGained(ev);
        break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
        WindowFocusLost(ev);
        break;
    case SDL_WINDOWEVENT_CLOSE:
        WindowClose(ev);
        break;
    case SDL_WINDOWEVENT_TAKE_FOCUS:
        WindowTakeFocus(ev);
        break;
    case SDL_WINDOWEVENT_HIT_TEST:
        WindowHitTest(ev);
        break;
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

void window::hide_system_cursor(bool value)
{
    SDL_ShowCursor(value ? SDL_DISABLE : SDL_ENABLE);
}

auto window::get_fullscreen() const -> bool
{
    return (SDL_GetWindowFlags(_window) & SDL_WINDOW_FULLSCREEN) != 0;
}

void window::set_fullscreen(bool value)
{
    SDL_SetWindowFullscreen(_window, value ? SDL_WINDOW_FULLSCREEN : 0);
}

auto window::get_title() const -> string
{
    return SDL_GetWindowTitle(_window);
}

void window::set_title(string const& value)
{
    SDL_SetWindowTitle(_window, value.c_str());
}

}
