// Copyright (c) 2025 Tobias Bohnen
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

namespace tcob::gfx::gl45 {

auto static GlobalUBO() -> gl_uniform_buffer&
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

    static gl_uniform_buffer globalUniformBuffer {sizeof(mat4) + sizeof(uvec2) + sizeof(ivec2) + sizeof(f32) + sizeof(u32)};
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
    offset += buffer.update(props.Viewport.Size, offset);
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
    glDisable(GL_STENCIL_TEST);
}

void gl_render_target::set_viewport(rect_i const& rect)
{
    if (_tex) {
        glViewport(rect.left(), _tex->info().Size.Height - rect.height() - rect.top(), rect.width(), rect.height());
    } else {
        glViewport(rect.left(), rect.top(), rect.width(), rect.height());
    }
}

void gl_render_target::enable_scissor(rect_i const& rect, i32 height) const
{
    if (rect.width() < 0 || rect.height() < 0) {
        return;
    }

    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.left(), height - rect.top() - rect.height(), rect.width(), rect.height());
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
    _tex->create(size, 1, texture::format::RGBA8);
    _frameBuffer->attach_texture(_tex);
}

auto gl_render_target::copy_to_image(rect_i const& rect) const -> image
{
    std::vector<u8> pixels(static_cast<usize>(rect.width() * rect.height() * 4));
    _frameBuffer->get_subimage(rect, pixels);
    auto retValue {image::Create(rect.Size, image::format::RGBA, pixels)};
    retValue.flip_vertically();
    return retValue;
}

void gl_render_target::bind_material(material const* mat) const
{
    if (!mat) { return; }

    if (mat->Texture.is_ready()) {
        glBindTextureUnit(0, mat->Texture->get_impl<gl_texture>()->ID);
    }

    if (mat->Shader.is_ready()) {
        glUseProgram(mat->Shader->get_impl<gl_shader>()->ID);
    } else {
        if (mat->Texture.is_ready()) {
            if (mat->Texture->info().Format == texture::format::R8) {
                glUseProgram(gl_context::DefaultFontShader);
            } else {
                glUseProgram(gl_context::DefaultTexturedShader);
            }
        } else {
            glUseProgram(gl_context::DefaultShader);
        }
    }

    usize offset {0};
    offset += _matUniformBuffer.update(mat->Color.to_float_array(), offset);
    offset += _matUniformBuffer.update(mat->PointSize, offset);
    _matUniformBuffer.bind_base(1);

    // set blend mode
    glEnable(GL_BLEND);
    glBlendFuncSeparate(
        convert_enum(mat->BlendFuncs.SourceColorBlendFunc), convert_enum(mat->BlendFuncs.DestinationColorBlendFunc),
        convert_enum(mat->BlendFuncs.SourceAlphaBlendFunc), convert_enum(mat->BlendFuncs.DestinationAlphaBlendFunc));
    glBlendEquation(convert_enum(mat->BlendEquation));

    glPointSize(mat->PointSize);

    // set stencil mode
    bool const needsStencil {(mat->StencilFunc != stencil_func::Always) || (mat->StencilOp != stencil_op::Keep)};
    if (needsStencil) {
        glEnable(GL_STENCIL_TEST);

        GLenum stencilFunc {};
        switch (mat->StencilFunc) {
        case stencil_func::Never: stencilFunc = GL_NEVER; break;
        case stencil_func::Less: stencilFunc = GL_LESS; break;
        case stencil_func::Equal: stencilFunc = GL_EQUAL; break;
        case stencil_func::LessEqual: stencilFunc = GL_LEQUAL; break;
        case stencil_func::Greater: stencilFunc = GL_GREATER; break;
        case stencil_func::NotEqual: stencilFunc = GL_NOTEQUAL; break;
        case stencil_func::GreaterEqual: stencilFunc = GL_GEQUAL; break;
        case stencil_func::Always: stencilFunc = GL_ALWAYS; break;
        }

        GLenum stencilOp {};
        switch (mat->StencilOp) {
        case stencil_op::Keep: stencilOp = GL_KEEP; break;
        case stencil_op::Zero: stencilOp = GL_ZERO; break;
        case stencil_op::Replace: stencilOp = GL_REPLACE; break;
        case stencil_op::Increase: stencilOp = GL_INCR; break;
        case stencil_op::Decrease: stencilOp = GL_DECR; break;
        case stencil_op::Invert: stencilOp = GL_INVERT; break;
        case stencil_op::IncreaseWrap: stencilOp = GL_INCR_WRAP; break;
        case stencil_op::DecreaseWrap: stencilOp = GL_DECR_WRAP; break;
        }

        glStencilMask(0xFF);
        glStencilFunc(stencilFunc, mat->StencilRef, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, stencilOp);
    } else {
        glDisable(GL_STENCIL_TEST);
    }
}

void gl_render_target::unbind_material() const
{
    glBindTextureUnit(0, 0);
    glUseProgram(0);
    glPointSize(1);
}

}
