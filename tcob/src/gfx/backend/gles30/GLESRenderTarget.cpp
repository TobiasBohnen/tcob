// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLESRenderTarget.hpp"

#include <glad/gles30.h>

#include "GLES30.hpp"
#include "GLESContext.hpp"
#include "GLESEnum.hpp"
#include "GLESShaderProgram.hpp"

#include "tcob/gfx/RenderTarget.hpp"

namespace tcob::gfx::gles30 {

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
        // TODO: not supported
    }
}

void gl_render_target::finalize_render() const
{
    _frameBuffer->bind_default();

    GLCHECK(glDisable(GL_BLEND));
}

void gl_render_target::set_viewport(rect_i const& rect)
{
    if (_tex) {
        GLCHECK(glViewport(rect.X, _tex->get_size().Height - rect.Height - rect.Y, rect.Width, rect.Height));
    } else {
        GLCHECK(glViewport(rect.X, rect.Y, rect.Width, rect.Height));
    }
}

void gl_render_target::enable_scissor(rect_i const& rect, i32 height) const
{
    if (rect.Width < 0 || rect.Height < 0) {
        return;
    }

    GLCHECK(glEnable(GL_SCISSOR_TEST));
    GLCHECK(glScissor(rect.left(), height - rect.top() - rect.Height, rect.Width, rect.Height));
}

void gl_render_target::disable_scissor() const
{
    GLCHECK(glDisable(GL_SCISSOR_TEST));
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
    std::vector<u8> pixels(static_cast<usize>(rect.Width * rect.Height * 4));
    _frameBuffer->get_subimage(rect, pixels, GL_RGBA);
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
        GLCHECK(glActiveTexture(GL_TEXTURE0));
        GLCHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, mat->Texture->get_impl<gl_texture>()->get_id()));
    }

    if (mat->Shader.is_ready()) {
        auto* shader {mat->Shader->get_impl<gl_shader>()};
        glUseProgram(shader->get_id());
        GLCHECK(glUniformBlockBinding(shader->get_id(), glGetUniformBlockIndex(shader->get_id(), "Material"), 1));
    } else {
        if (mat->Texture.is_ready()) {
            if (mat->Texture->get_format() == texture::format::R8) {
                GLCHECK(glUseProgram(gl_context::DefaultFontShader));
                GLCHECK(glUniformBlockBinding(gl_context::DefaultFontShader, glGetUniformBlockIndex(gl_context::DefaultFontShader, "Material"), 1));
            } else {
                GLCHECK(glUseProgram(gl_context::DefaultTexturedShader));
                GLCHECK(glUniformBlockBinding(gl_context::DefaultTexturedShader, glGetUniformBlockIndex(gl_context::DefaultTexturedShader, "Material"), 1));
            }
        } else {
            GLCHECK(glUseProgram(gl_context::DefaultShader));
            GLCHECK(glUniformBlockBinding(gl_context::DefaultShader, glGetUniformBlockIndex(gl_context::DefaultShader, "Material"), 1));
        }
    }

    usize offset {0};
    offset += _matUniformBuffer.update(mat->Color.as_float_array(), offset);
    offset += _matUniformBuffer.update(mat->PointSize, offset);
    _matUniformBuffer.bind_base(1);

    // set blend mode
    GLCHECK(glEnable(GL_BLEND));
    glBlendFuncSeparate(
        convert_enum(mat->BlendFuncs.SourceColorBlendFunc), convert_enum(mat->BlendFuncs.DestinationColorBlendFunc),
        convert_enum(mat->BlendFuncs.SourceAlphaBlendFunc), convert_enum(mat->BlendFuncs.DestinationAlphaBlendFunc));
    GLCHECK(glBlendEquation(convert_enum(mat->BlendEquation)));
}

void gl_render_target::unbind_material() const
{
    GLCHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, 0));
    GLCHECK(glUseProgram(0));
}

}
