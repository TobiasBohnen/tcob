// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLES20RenderTarget.hpp"

#include <glad/gles20.h>

#include "GLES20.hpp"
#include "GLES20Context.hpp"
#include "GLES20Enum.hpp"
#include "GLES20ShaderProgram.hpp"

namespace tcob::gfx::gles20 {

gl_render_target::gl_render_target(texture* tex)
    : _tex {tex}
    , _frameBuffer {std::make_unique<gl_framebuffer>()}
{
}

void gl_render_target::prepare_render(render_properties const& props)
{
    _props = props;

    if (props.UseDefaultFramebuffer) {
        _frameBuffer->bind_default();
    } else {
        _frameBuffer->bind();
    }

    set_viewport(props.Viewport);

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
        GLCHECK(glViewport(rect.left(), _tex->info().Size.Height - rect.height() - rect.top(), rect.width(), rect.height()));
    } else {
        GLCHECK(glViewport(rect.left(), rect.top(), rect.width(), rect.height()));
    }
}

void gl_render_target::enable_scissor(rect_i const& rect, i32 height) const
{
    if (rect.width() < 0 || rect.height() < 0) {
        return;
    }

    GLCHECK(glEnable(GL_SCISSOR_TEST));
    GLCHECK(glScissor(rect.left(), height - rect.top() - rect.height(), rect.width(), rect.height()));
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
    std::vector<u8> pixels(static_cast<usize>(rect.width() * rect.height() * 4));
    _frameBuffer->get_subimage(rect, pixels, GL_RGBA);
    auto retValue {image::Create(rect.Size, image::format::RGBA, pixels)};
    retValue.flip_vertically();
    return retValue;
}

void gl_render_target::bind_material(material const* mat) const
{
    if (!mat) { return; }

    if (mat->Texture.is_ready()) {
        GLCHECK(glActiveTexture(GL_TEXTURE0));
        GLCHECK(glBindTexture(GL_TEXTURE_2D, mat->Texture->get_impl<gl_texture>()->get_id()));
    }

    u32 shaderId {};

    if (mat->Shader.is_ready()) {
        auto* shader {mat->Shader->get_impl<gl_shader>()};
        shaderId = shader->get_id();
    } else {
        if (mat->Texture.is_ready()) {
            if (mat->Texture->info().Format == texture::format::R8) {
                shaderId = gl_context::DefaultFontShader;
            } else {
                shaderId = gl_context::DefaultTexturedShader;
            }
        } else {
            shaderId = gl_context::DefaultShader;
        }
    }

    GLCHECK(glUseProgram(shaderId));
    glUniformMatrix4fv(glGetUniformLocation(shaderId, "camera"), 1, GL_FALSE, _props.ViewMatrix.data());
    glUniform2i(glGetUniformLocation(shaderId, "view_size"), _props.Viewport.Size.Width, _props.Viewport.Size.Height);
    glUniform2i(glGetUniformLocation(shaderId, "mouse_pos"), _props.MousePosition.X, _props.MousePosition.Y);
    glUniform1f(glGetUniformLocation(shaderId, "time"), _props.Time);
    glUniform1i(glGetUniformLocation(shaderId, "debug"), _props.Debug);

    glUniform4f(glGetUniformLocation(shaderId, "matColor"), mat->Color.R / 255.f, mat->Color.G / 255.f, mat->Color.B / 255.f, mat->Color.A / 255.f);
    glUniform1f(glGetUniformLocation(shaderId, "matPointSize"), mat->PointSize);

    // set blend mode
    GLCHECK(glEnable(GL_BLEND));
    glBlendFuncSeparate(
        convert_enum(mat->BlendFuncs.SourceColorBlendFunc), convert_enum(mat->BlendFuncs.DestinationColorBlendFunc),
        convert_enum(mat->BlendFuncs.SourceAlphaBlendFunc), convert_enum(mat->BlendFuncs.DestinationAlphaBlendFunc));
    GLCHECK(glBlendEquation(convert_enum(mat->BlendEquation)));
}

void gl_render_target::unbind_material() const
{
    GLCHECK(glBindTexture(GL_TEXTURE_2D, 0));
    GLCHECK(glUseProgram(0));
}

}
