// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLES20RenderSystem.hpp"

#include "GLES20Framebuffer.hpp"
#include "GLES20RenderTarget.hpp"
#include "GLES20ShaderProgram.hpp"
#include "GLES20Texture.hpp"
#include "GLES20VertexArray.hpp"
#include "GLES20Window.hpp"
#include "nanovg/GLES20Canvas.hpp"

namespace tcob::gfx::gles20 {

auto gl_render_system::name() const -> string
{
    return "OPENGLES20";
}

auto gl_render_system::device_name() const -> string
{
    auto const* str {glGetString(GL_RENDERER)};
    return str ? reinterpret_cast<char const*>(str) : "";
}

auto gl_render_system::caps() const -> capabilities
{
    capabilities retValue;

    retValue.PointSize.Range       = {0.0f, 4096.0f};
    retValue.PointSize.Granularity = 0.01f;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &retValue.Texture.MaxSize);
    retValue.Texture.MaxLayers = 1;

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

auto gl_render_system::create_uniform_buffer(usize /* size */) -> std::unique_ptr<render_backend::uniform_buffer_base>
{
    return nullptr;
}

auto gl_render_system::create_vertex_array(buffer_usage_hint usage) -> std::unique_ptr<render_backend::vertex_array_base>
{
    return std::make_unique<gl_vertex_array>(usage);
}

auto gl_render_system::create_window(size_i size) -> std::unique_ptr<render_backend::window_base>
{
    return std::make_unique<gl_window>(size);
}

} // namespace gfx
