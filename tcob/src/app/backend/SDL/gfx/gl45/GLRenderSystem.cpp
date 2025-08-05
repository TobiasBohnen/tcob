// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLRenderSystem.hpp"

#include <array>
#include <memory>

#include <glad/gl45.h>

#include "../SDLWindow.hpp"
#include "GLCanvas.hpp"
#include "GLRenderTarget.hpp"
#include "GLShaderProgram.hpp"
#include "GLTexture.hpp"
#include "GLUniformBuffer.hpp"
#include "GLVertexArray.hpp"
#include "GLWindow.hpp"

#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx::gl45 {

auto gl_render_system::name() const -> string
{
    return "OPENGL45";
}

auto gl_render_system::device_name() const -> string
{
    auto const* str {glGetString(GL_RENDERER)};
    return str ? reinterpret_cast<char const*>(str) : "";
}

auto gl_render_system::capabilities() const -> render_capabilities
{
    render_capabilities retValue;

    std::array<f32, 2> val0 {};
    glGetFloatv(GL_POINT_SIZE_RANGE, val0.data());
    retValue.PointSize.Range = {val0[0], val0[1]};
    glGetFloatv(GL_POINT_SIZE_GRANULARITY, &retValue.PointSize.Granularity);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &retValue.Texture.MaxSize);
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &retValue.Texture.MaxLayers);

    return retValue;
}

auto gl_render_system::rtt_coords() const -> rect_f
{
    return {0, 0, 1, -1};
}

auto gl_render_system::create_canvas() -> std::unique_ptr<render_backend::canvas_base>
{
    return std::make_unique<gl_canvas>();
}

auto gl_render_system::create_render_target(texture* tex) -> std::unique_ptr<render_backend::render_target_base>
{
    return std::make_unique<gl_render_target>(tex);
}

auto gl_render_system::create_shader() -> std::unique_ptr<render_backend::shader_base>
{
    return std::make_unique<gl_shader>();
}

auto gl_render_system::create_texture() -> std::unique_ptr<render_backend::texture_base>
{
    return std::make_unique<gl_texture>();
}

auto gl_render_system::create_uniform_buffer(usize size) -> std::unique_ptr<render_backend::uniform_buffer_base>
{
    return std::make_unique<gl_uniform_buffer>(size);
}

auto gl_render_system::create_vertex_array(buffer_usage_hint usage) -> std::unique_ptr<render_backend::vertex_array_base>
{
    return std::make_unique<gl_vertex_array>(usage);
}

auto gl_render_system::create_window(size_i size) -> std::unique_ptr<gfx::window>
{
    return std::make_unique<sdl_window>(std::make_unique<gl_window>(size));
}

} // namespace gfx
