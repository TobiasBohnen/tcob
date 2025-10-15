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
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx {

window::window(std::unique_ptr<render_backend::window_base> windowBase, asset_owner_ptr<texture> const& texture)
    : render_target {texture.ptr()}
    , FullScreen {make_prop_fn<bool, &window::get_fullscreen, &window::set_fullscreen>(this)}
    , Title {make_prop_fn<string, &window::get_title, &window::set_title>(this)}
    , VSync {make_prop_fn<bool,
                          [](window const& w) { return w._impl->get_vsync(); },
                          [](window& w, bool value) { w._impl->set_vsync(value); }>(this)}
    , _texture {texture}
    , _impl {std::move(windowBase)}
{
    Cursor.Changed.connect([this](auto const& value) { SystemCursorEnabled = !value.is_ready(); });
    SystemCursorEnabled(true);

    Shader.Changed.connect([this](auto const& value) { _material->first_pass().Shader = value; });

    _material->first_pass().Texture = _texture;
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

void window::init_renderer(quad const& q)
{
    _renderer.set_geometry(q, &_material->first_pass());
}

}
