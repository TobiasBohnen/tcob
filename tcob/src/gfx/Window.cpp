// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Window.hpp"

#include <SDL.h>

#include <utility>

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
    , Shader {{[&]() { return _material->Shader; },
               [&](auto const& value) { _material->Shader = value; }}}
    , _texture {texture}
    , _impl {std::move(window)}
{
    Cursor.Changed.connect([&](auto const& value) { SystemCursorEnabled = !value.is_ready(); });
    SystemCursorEnabled(true);
    SystemCursorEnabled.Changed.connect([&](bool value) { SDL_ShowCursor(value ? SDL_ENABLE : SDL_DISABLE); });

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

        SDL_SetWindowIcon(_impl->get_handle(), surface);
        SDL_FreeSurface(surface);
    }
}

auto window::has_focus() const -> bool
{
    return (SDL_GetWindowFlags(_impl->get_handle()) & SDL_WINDOW_MOUSE_FOCUS)
        && (SDL_GetWindowFlags(_impl->get_handle()) & SDL_WINDOW_INPUT_FOCUS);
}

void window::grab_input(bool grab)
{
    SDL_SetWindowGrab(_impl->get_handle(), grab ? SDL_TRUE : SDL_FALSE);
}

void window::on_clear(color c) const
{
    _impl->clear(c);
}

void window::set_size(size_i newSize)
{
    if (newSize != get_size()) {
        SDL_SetWindowSize(_impl->get_handle(), newSize.Width, newSize.Height);
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

    switch (sdlEv->window.event) {
    case SDL_WINDOWEVENT_SHOWN: Shown(ev); break;
    case SDL_WINDOWEVENT_HIDDEN: Hidden(ev); break;
    case SDL_WINDOWEVENT_EXPOSED: Exposed(ev); break;
    case SDL_WINDOWEVENT_MOVED: Moved(ev); break;
    case SDL_WINDOWEVENT_RESIZED:
        set_size({ev.Data1, ev.Data2});
        Resized(ev);
        break;
    case SDL_WINDOWEVENT_SIZE_CHANGED: SizeChanged(ev); break;
    case SDL_WINDOWEVENT_MINIMIZED: Minimized(ev); break;
    case SDL_WINDOWEVENT_MAXIMIZED: Maximized(ev); break;
    case SDL_WINDOWEVENT_RESTORED: Restored(ev); break;
    case SDL_WINDOWEVENT_ENTER: Enter(ev); break;
    case SDL_WINDOWEVENT_LEAVE: Leave(ev); break;
    case SDL_WINDOWEVENT_FOCUS_GAINED: FocusGained(ev); break;
    case SDL_WINDOWEVENT_FOCUS_LOST: FocusLost(ev); break;
    case SDL_WINDOWEVENT_CLOSE: Close(ev); break;
    case SDL_WINDOWEVENT_TAKE_FOCUS: TakeFocus(ev); break;
    case SDL_WINDOWEVENT_HIT_TEST: HitTest(ev); break;
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
    auto* window {_impl->get_handle()};
    SDL_SetWindowFullscreen(window, value ? SDL_WINDOW_FULLSCREEN : 0);
    if (!value) {
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_SetWindowBordered(window, SDL_TRUE);
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
