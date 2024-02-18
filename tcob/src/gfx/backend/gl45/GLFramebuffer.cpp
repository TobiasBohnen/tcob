// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLFramebuffer.hpp"

#include <cassert>
#include <glad/gl45.h>

#include "GLTexture.hpp"

namespace tcob::gfx::gl45 {
gl_framebuffer::gl_framebuffer()
{
    glCreateFramebuffers(1, &ID);
    glCreateRenderbuffers(1, &_rbo);
}

gl_framebuffer::~gl_framebuffer()
{
    destroy();
}

void gl_framebuffer::do_destroy()
{
    glDeleteFramebuffers(1, &ID);
    glDeleteRenderbuffers(1, &_rbo);
    _rbo = 0;
}

void gl_framebuffer::bind() const
{
    assert(ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
}

void gl_framebuffer::bind_default() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void gl_framebuffer::attach_texture(texture const* tex)
{
    assert(ID);

    auto const size {tex->get_size()};
    _texID = tex->get_impl<gl_texture>()->get_id();
    glNamedFramebufferTextureLayer(ID, GL_COLOR_ATTACHMENT0, _texID, 0, 0);

    glNamedFramebufferRenderbuffer(ID, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo);
    glNamedRenderbufferStorage(_rbo, GL_DEPTH24_STENCIL8, size.Width, size.Height);

    // Set the list of draw buffers.
    std::array<GLenum, 1> const buffers {GL_COLOR_ATTACHMENT0};
    glNamedFramebufferDrawBuffers(ID, static_cast<GLsizei>(buffers.size()), buffers.data());

    [[maybe_unused]] GLenum const complete {glCheckNamedFramebufferStatus(ID, GL_FRAMEBUFFER)};
    assert(complete == GL_FRAMEBUFFER_COMPLETE);
}

void gl_framebuffer::get_subimage(rect_i const& rect, std::span<u8> pixels)
{
    assert(ID);
    glGetTextureSubImage(
        _texID,
        0,
        rect.left(), rect.top(), 0, rect.Width, rect.Height,
        1,
        GL_RGBA, GL_UNSIGNED_BYTE,
        static_cast<i32>(pixels.size_bytes()), pixels.data());
}

auto gl_framebuffer::read_pixel(point_i pos) const -> color
{
    bind();
    std::array<u8, 4> data {};
    glReadPixels(pos.X, pos.Y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    bind_default();
    return color {data[0], data[1], data[2], data[3]};
}

void gl_framebuffer::clear(color c) const
{
    assert(ID);
    vec4 color {static_cast<f32>(c.R) / 255.0f, static_cast<f32>(c.G) / 255.0f, static_cast<f32>(c.B) / 255.0f, static_cast<f32>(c.A) / 255.0f};
    glClearNamedFramebufferfv(ID, GL_COLOR, 0, color.data());
    glClearNamedFramebufferfi(ID, GL_DEPTH_STENCIL, 0, 1, 0);
}
}
