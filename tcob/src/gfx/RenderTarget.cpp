// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/RenderTarget.hpp"

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Stats.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/RenderSystem.hpp"

namespace tcob::gfx {

render_target::render_target(texture* tex)
    : Size {{[&]() { return get_size(); },
             [&](auto const& value) { set_size(value); }}}
    , _impl {locate_service<render_system>().create_render_target(tex)}
{
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
    auto const& stats {locate_service<render_system>().get_stats()};

    auto& cam {*Camera};
    _impl->prepare_render(
        {.ViewMatrix            = cam.get_matrix(),
         .Viewport              = rect_i {cam.get_viewport()},
         .MousePosition         = input::system::GetMousePosition(),
         .Time                  = stats.get_time(),
         .Debug                 = debug,
         .UseDefaultFramebuffer = false});
}

void render_target::finalize_render() const
{
    _impl->finalize_render();
}

void render_target::bind_material(material* mat) const
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

void render_target::enable_scissor(rect_i const& rect) const
{
    _impl->enable_scissor(rect, get_size().Height);
}

void render_target::disable_scissor() const
{
    _impl->disable_scissor();
}

auto render_target::copy_to_image() const -> image
{
    return _impl->copy_to_image({point_i::Zero, get_size()});
}

void render_target::set_size(size_i size)
{
    (*Camera).set_size(size_f {size}); // FIXME: don't change camera size

    if (size.Width <= 0 || size.Height <= 0) { return; }

    _impl->on_resize(size);
}

////////////////////////////////////////////////////////////

default_render_target::default_render_target()
    : render_target {nullptr}
{
}

auto default_render_target::get_size() const -> size_i { return _size; }

void default_render_target::set_size(size_i newsize)
{
    if (newsize != _size) {
        _size = newsize;
        render_target::set_size(_size);
    }
}

void default_render_target::prepare_render(bool)
{
    auto const& stats {locate_service<render_system>().get_stats()};

    constexpr mat4 Matrix {1.0f, 0.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f,
                           0.0f, 0.0f, 0.0f, 1.0f};

    get_impl<render_backend::render_target_base>()->prepare_render({
        .ViewMatrix            = Matrix,
        .Viewport              = {point_i::Zero, get_size()},
        .MousePosition         = input::system::GetMousePosition(),
        .Time                  = stats.get_time(),
        .Debug                 = false,
        .UseDefaultFramebuffer = true,
    });
}
}
