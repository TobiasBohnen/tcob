// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/RenderTarget.hpp"

#include <cassert>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/Stats.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Camera.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"
#include "tcob/gfx/Texture.hpp"
#include "tcob/gfx/Window.hpp"

namespace tcob::gfx {

render_target::render_target(texture* tex)
    : Size {{[this]() { return get_size(); },
             [this](auto const& value) { set_size(value); }}}
    , _impl {locate_service<render_system>().create_render_target(tex)}
    , _camera {*this}
{
}

render_target::~render_target() = default;

auto render_target::camera() -> gfx::camera&
{
    return _camera;
}

void render_target::clear() const
{
    clear(ClearColor);
}

void render_target::clear(color c) const
{
    _impl->clear(c);
    on_clear(c);
}

void render_target::prepare_render(bool debug)
{
    auto const& stats {locate_service<render_system>().stats()};

    if (ScissorRect) {
        _impl->enable_scissor(*ScissorRect, get_size().Height);
    } else {
        _impl->disable_scissor();
    }

    _impl->prepare_render(
        {.ViewMatrix            = _camera.matrix(),
         .Viewport              = rect_i {_camera.viewport()},
         .MousePosition         = locate_service<input::system>().mouse().get_position(),
         .Time                  = stats.current_time(),
         .Debug                 = debug,
         .UseDefaultFramebuffer = false});
}

void render_target::finalize_render() const
{
    _impl->finalize_render();
}

void render_target::bind_material(material const* mat) const
{
    _impl->bind_material(mat);
}

void render_target::unbind_material() const
{
    _impl->unbind_material();
}

void render_target::on_clear(color /*c*/) const
{
}

auto render_target::copy_to_image() const -> image
{
    return _impl->copy_to_image({point_i::Zero, get_size()});
}

void render_target::set_size(size_i size)
{
    if (size.Width <= 0 || size.Height <= 0) { return; }

    _impl->on_resize(size);
}

////////////////////////////////////////////////////////////

default_render_target::default_render_target(window* win)
    : render_target {nullptr}
    , _window {win}
{
}

auto default_render_target::get_size() const -> size_i { return _window ? _window->Size() : size_i::Zero; }

void default_render_target::set_size(size_i /* newsize */)
{
    assert(false);
}

void default_render_target::prepare_render(bool)
{
    auto const& stats {locate_service<render_system>().stats()};

    static constexpr mat4 Matrix {1.0f, 0.0f, 0.0f, 0.0f,
                                  0.0f, 1.0f, 0.0f, 0.0f,
                                  0.0f, 0.0f, 1.0f, 0.0f,
                                  0.0f, 0.0f, 0.0f, 1.0f};

    get_impl<render_backend::render_target_base>()->prepare_render({
        .ViewMatrix            = Matrix,
        .Viewport              = {point_i::Zero, get_size()},
        .MousePosition         = locate_service<input::system>().mouse().get_position(),
        .Time                  = stats.current_time(),
        .Debug                 = false,
        .UseDefaultFramebuffer = true,
    });
}
}
