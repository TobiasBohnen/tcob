// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLES30RenderTarget.hpp"

#include <glad/gles30.h>
#include <memory>
#include <vector>

#include "GLES30.hpp"
#include "GLES30Context.hpp"
#include "GLES30Enum.hpp"
#include "GLES30Framebuffer.hpp"
#include "GLES30ShaderProgram.hpp"
#include "GLES30Texture.hpp"
#include "GLES30UniformBuffer.hpp"

#include "tcob/core/Color.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx::gles30 {

static auto GlobalUBO() -> gl_uniform_buffer&
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
        _tex->resize(size, 1, texture::format::RGBA8);
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
        GLCHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, mat->Texture->get_impl<gl_texture>()->ID));
    }

    if (mat->Shader.is_ready()) {
        auto* shader {mat->Shader->get_impl<gl_shader>()};
        glUseProgram(shader->ID);
        GLCHECK(glUniformBlockBinding(shader->ID, glGetUniformBlockIndex(shader->ID, "Material"), 1));
    } else {
        if (mat->Texture.is_ready()) {
            if (mat->Texture->info().Format == texture::format::R8) {
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
    offset += _matUniformBuffer.update(mat->Color.to_float_array(), offset);
    offset += _matUniformBuffer.update(mat->PointSize, offset);
    _matUniformBuffer.bind_base(1);

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
    GLCHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, 0));
    GLCHECK(glUseProgram(0));
}

}
