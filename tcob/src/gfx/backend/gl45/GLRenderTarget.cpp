// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLRenderTarget.hpp"

#include <glad/gl45.h>

#include "GLContext.hpp"
#include "GLEnum.hpp"
#include "GLShaderProgram.hpp"
#include "GLTexture.hpp"

#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/UniformBuffer.hpp"

namespace tcob::gfx::gl45 {

auto static GlobalUBO() -> uniform_buffer&
{
    /*
    layout(std140, binding = 0)uniform Globals
    {
        mat4 camera;
        uvec2 view_size;
        ivec2 mouse_pos;
        float time;
        bool debug;
    };
    */

    static uniform_buffer globalUniformBuffer {sizeof(mat4) + sizeof(uvec2) + sizeof(ivec2) + sizeof(f32) + sizeof(u32)};
    return globalUniformBuffer;
}

gl_render_target::gl_render_target(texture* tex)
    : _tex {tex}
    , _frameBuffer {std::make_unique<gl_framebuffer>()}
{
}

void gl_render_target::prepare_render(render_properties const& props)
{
    if (props.UseDefaultFramebuffer) {
        _frameBuffer->bind_default();
    } else {
        _frameBuffer->bind();
    }

    set_viewport(props.Viewport);

    // setup UBO
    auto& buffer {GlobalUBO()};
    usize offset {0};

    offset += buffer.update(props.ViewMatrix, offset);
    offset += buffer.update(props.Viewport.get_size(), offset);
    offset += buffer.update(props.MousePosition, offset);
    offset += buffer.update(props.Time, offset);
    offset += buffer.update(props.Debug, offset);

    buffer.bind_base(0);

    // set polygon mode
    if (props.Debug) {
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_BLEND);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glEnable(GL_LINE_SMOOTH);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void gl_render_target::finalize_render() const
{
    _frameBuffer->bind_default();

    glDisable(GL_BLEND);
}

void gl_render_target::set_viewport(rect_i const& rect)
{
    if (_tex) {
        glViewport(rect.X, _tex->get_size().Height - rect.Height - rect.Y, rect.Width, rect.Height);
    } else {
        glViewport(rect.X, rect.Y, rect.Width, rect.Height);
    }
}

void gl_render_target::enable_scissor(rect_i const& rect, i32 height) const
{
    if (rect.Width < 0 || rect.Height < 0) {
        return;
    }

    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.left(), height - rect.top() - rect.Height, rect.Width, rect.Height);
}

void gl_render_target::disable_scissor() const
{
    glDisable(GL_SCISSOR_TEST);
}

void gl_render_target::clear(color c) const
{
    _frameBuffer->clear(c);
}

void gl_render_target::on_resize(size_i size)
{
    if (_tex) {
        _tex->create(size, 1, texture::format::RGBA8);
        _frameBuffer->attach_texture(_tex);
    }
}

auto gl_render_target::copy_to_image(rect_i const& rect) const -> image
{
    std::vector<u8> pixels(rect.Width * rect.Height * 4);
    _frameBuffer->get_subimage(rect, pixels);
    auto retValue {image::Create(rect.get_size(), image::format::RGBA, pixels)};
    retValue.flip_vertically();
    return retValue;
}

void gl_render_target::bind_material(material* mat) const
{
    if (!mat) {
        return;
    }

    if (mat->Texture.is_ready()) {
        glBindTextureUnit(0, mat->Texture->get_impl<gl_texture>()->get_id());
    }

    if (mat->Shader.is_ready()) {
        glUseProgram(mat->Shader->get_impl<gl_shader>()->get_id());
    } else {
        if (mat->Texture.is_ready()) {
            if (mat->Texture->get_format() == texture::format::R8) {
                glUseProgram(gl_context::DefaultFontShader);
            } else {
                glUseProgram(gl_context::DefaultTexturedShader);
            }
        } else {
            glUseProgram(gl_context::DefaultShader);
        }
    }

    // set blend mode
    glEnable(GL_BLEND);
    glBlendFuncSeparate(
        convert_enum(mat->BlendFuncs.SourceColorBlendFunc), convert_enum(mat->BlendFuncs.DestinationColorBlendFunc),
        convert_enum(mat->BlendFuncs.SourceAlphaBlendFunc), convert_enum(mat->BlendFuncs.DestinationAlphaBlendFunc));
    glBlendEquation(convert_enum(mat->BlendEquation));

    glPointSize(mat->PointSize);
}

void gl_render_target::unbind_material() const
{
    glBindTextureUnit(0, 0);
    glUseProgram(0);
    glPointSize(1);
}

}
