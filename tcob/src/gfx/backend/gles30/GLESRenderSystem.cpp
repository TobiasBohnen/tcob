// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLESRenderSystem.hpp"

#include "GLESFramebuffer.hpp"
#include "GLESRenderTarget.hpp"
#include "GLESShaderProgram.hpp"
#include "GLESTexture.hpp"
#include "GLESUniformBuffer.hpp"
#include "GLESVertexArray.hpp"
#include "GLESWindow.hpp"
#include "nanovg/GLESCanvas.hpp"

namespace tcob::gfx::gles30 {

auto gl_render_system::get_name() const -> string
{
    return "OPENGLES30";
}

auto gl_render_system::get_device_name() const -> string
{
    auto const* str {glGetString(GL_RENDERER)};
    return str ? reinterpret_cast<char const*>(str) : "";
}

auto gl_render_system::get_capabilities() const -> capabilities
{
    capabilities retValue;

    retValue.PointSizeRange       = {0.0f, 4096.0f};
    retValue.PointSizeGranularity = 0.01f;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &retValue.MaxTextureSize);
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &retValue.MaxArrayTextureLayers);

    return retValue;
}

auto gl_render_system::get_rtt_coords() const -> rect_f
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

auto gl_render_system::create_window(size_i size) -> std::unique_ptr<render_backend::window_base>
{
    return std::make_unique<gl_window>(size);
}

} // namespace gfx
