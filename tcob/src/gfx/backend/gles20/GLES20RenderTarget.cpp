// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLES20RenderTarget.hpp"

#include <glad/gles20.h>
#include <memory>
#include <vector>

#include "GLES20.hpp"
#include "GLES20Context.hpp"
#include "GLES20Enum.hpp"
#include "GLES20Framebuffer.hpp"
#include "GLES20ShaderProgram.hpp"
#include "GLES20Texture.hpp"

#include "tcob/core/Color.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Texture.hpp"

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
    GLCHECK(glDisable(GL_STENCIL_TEST));
}

void gl_render_target::set_viewport(rect_i const& rect)
{
    if (_tex) {
        GLCHECK(glViewport(rect.left(), _tex->info().Size.Height - rect.height() - rect.top(), rect.width(), rect.height()));
    } else {
        GLCHECK(glViewport(rect.left(), rect.top(), rect.width(), rect.height()));
    }
}

void gl_render_target::enable_scissor(rect_i const& rect) const
{
    if (rect.width() < 0 || rect.height() < 0) {
        return;
    }

    GLCHECK(glEnable(GL_SCISSOR_TEST));
    i32 const height {_tex->info().Size.Height};
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
        GLCHECK(glBindTexture(GL_TEXTURE_2D, mat->Texture->get_impl<gl_texture>()->ID));
    }

    u32 shaderId {};

    if (mat->Shader.is_ready()) {
        auto* shader {mat->Shader->get_impl<gl_shader>()};
        shaderId = shader->ID;
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

    // set stencil mode
    bool const needsStencil {(mat->StencilFunc != stencil_func::Always) || (mat->StencilOp != stencil_op::Keep)};
    if (needsStencil) {
        GLCHECK(glEnable(GL_STENCIL_TEST));

        GLenum stencilFunc {};
        switch (mat->StencilFunc) {
        case stencil_func::Never:        stencilFunc = GL_NEVER; break;
        case stencil_func::Less:         stencilFunc = GL_LESS; break;
        case stencil_func::Equal:        stencilFunc = GL_EQUAL; break;
        case stencil_func::LessEqual:    stencilFunc = GL_LEQUAL; break;
        case stencil_func::Greater:      stencilFunc = GL_GREATER; break;
        case stencil_func::NotEqual:     stencilFunc = GL_NOTEQUAL; break;
        case stencil_func::GreaterEqual: stencilFunc = GL_GEQUAL; break;
        case stencil_func::Always:       stencilFunc = GL_ALWAYS; break;
        }

        GLenum stencilOp {};
        switch (mat->StencilOp) {
        case stencil_op::Keep:         stencilOp = GL_KEEP; break;
        case stencil_op::Zero:         stencilOp = GL_ZERO; break;
        case stencil_op::Replace:      stencilOp = GL_REPLACE; break;
        case stencil_op::Increase:     stencilOp = GL_INCR; break;
        case stencil_op::Decrease:     stencilOp = GL_DECR; break;
        case stencil_op::Invert:       stencilOp = GL_INVERT; break;
        case stencil_op::IncreaseWrap: stencilOp = GL_INCR_WRAP; break;
        case stencil_op::DecreaseWrap: stencilOp = GL_DECR_WRAP; break;
        }

        GLCHECK(glStencilMask(0xFF));
        GLCHECK(glStencilFunc(stencilFunc, mat->StencilRef, 0xFF));
        GLCHECK(glStencilOp(GL_KEEP, GL_KEEP, stencilOp));
    } else {
        GLCHECK(glDisable(GL_STENCIL_TEST));
    }
}

void gl_render_target::unbind_material() const
{
    GLCHECK(glBindTexture(GL_TEXTURE_2D, 0));
    GLCHECK(glUseProgram(0));
}

}
