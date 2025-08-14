// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Window.hpp"

#include <cassert>
#include <memory>
#include <utility>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx {

window::window(std::unique_ptr<render_backend::window_base> window, assets::asset_owner_ptr<texture> const& texture)
    : render_target {texture.ptr()}
    , FullScreen {{[this] { return get_fullscreen(); },
                   [this](auto const& value) { set_fullscreen(value); }}}
    , Title {{[this] { return get_title(); },
              [this](auto const& value) { set_title(value); }}}
    , VSync {{[this] { return _impl->get_vsync(); },
              [this](auto const& value) { _impl->set_vsync(value); }}}
    , _texture {texture}
    , _impl {std::move(window)}
{
    Cursor.Changed.connect([this](auto const& value) { SystemCursorEnabled = !value.is_ready(); });
    SystemCursorEnabled(true);

    Shader.Changed.connect([this](auto const& value) { _material->Shader = value; });

    _material->Texture = _texture;
    _renderer.set_material(_material.ptr());
}

window::~window() = default;

auto window::bounds() const -> rect_i
{
    return {point_i::Zero, Size};
}

void window::on_clear(color c) const
{
    _impl->clear(c);
}

void window::draw_to(render_target& target)
{
    _impl->set_viewport({point_i::Zero, get_size()});
    _renderer.render_to_target(target);

    if (*Cursor) {
        Cursor->draw_to(target);
    }
}

void window::swap_buffer() const
{
    _impl->swap_buffer();
}

auto window::get_impl() const -> render_backend::window_base*
{
    return _impl.get();
}

auto window::renderer() -> quad_renderer&
{
    return _renderer;
}

}
