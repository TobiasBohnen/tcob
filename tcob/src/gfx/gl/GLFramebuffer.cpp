// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/gl/GLFramebuffer.hpp>

#include <cassert>
#include <glad/gl.h>

namespace tcob::gl {
Framebuffer::Framebuffer()
{
    glCreateFramebuffers(1, &ID);
    glCreateRenderbuffers(1, &_rbo);
}

Framebuffer::~Framebuffer()
{
    destroy();
}

void Framebuffer::do_destroy()
{
    glDeleteFramebuffers(1, &ID);
    glDeleteRenderbuffers(1, &_rbo);
    _rbo = 0;
}

void Framebuffer::bind() const
{
    assert(ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
}

void Framebuffer::attach_texture(const Texture2D* tex)
{
    assert(ID);
    glNamedFramebufferTexture(ID, GL_COLOR_ATTACHMENT0, tex->ID, 0);

    glNamedFramebufferRenderbuffer(ID, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo);
    glNamedRenderbufferStorage(_rbo, GL_DEPTH24_STENCIL8, tex->size().Width, tex->size().Height);

    // Set the list of draw buffers.
    const std::array<GLenum, 1> buffers { GL_COLOR_ATTACHMENT0 };
    glNamedFramebufferDrawBuffers(ID, 1, buffers.data()); // "1" is the size of DrawBuffers

    const GLenum complete { glCheckNamedFramebufferStatus(ID, GL_FRAMEBUFFER) };
    assert(complete == GL_FRAMEBUFFER_COMPLETE);
    _size = tex->size();
}

void Framebuffer::blit_to(u32 target)
{
    glBlitNamedFramebuffer(ID, target,
        0, 0, _size.Width, _size.Height,
        0, 0, _size.Width, _size.Height,
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
        GL_NEAREST);
}

void Framebuffer::BindDefault()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::clear(const Color& c) const
{
    assert(ID);
    vec4 color { c.R / 255.f, c.G / 255.f, c.B / 255.f, c.A / 255.f };
    glClearNamedFramebufferfv(ID, GL_COLOR, 0, color.data());
    glClearNamedFramebufferfi(ID, GL_DEPTH_STENCIL, 0, 1, 0);
}
}