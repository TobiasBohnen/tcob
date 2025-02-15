#include "GLES20Framebuffer.hpp"

#include <cassert>

#include "GLES20.hpp"
#include "GLES20Texture.hpp"

namespace tcob::gfx::gles20 {

gl_framebuffer::gl_framebuffer()
{
    GLCHECK(glGenFramebuffers(1, &ID));
    GLCHECK(glGenRenderbuffers(1, &_rbo));
}

gl_framebuffer::~gl_framebuffer()
{
    destroy();
}

void gl_framebuffer::do_destroy()
{
    GLCHECK(glDeleteFramebuffers(1, &ID));
    GLCHECK(glDeleteRenderbuffers(1, &_rbo));
    _rbo = 0;
}

void gl_framebuffer::bind() const
{
    assert(ID);
    GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, ID));
    GLCHECK(glBindRenderbuffer(GL_RENDERBUFFER, _rbo));
}

void gl_framebuffer::bind_default() const
{
    GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void gl_framebuffer::attach_texture(texture const* tex)
{
    attach_texture(tex->get_impl<gl_texture>(), 0);
}

void gl_framebuffer::attach_texture(gl_texture const* tex, u32 /*depth*/)
{
    bind();

    auto const size {tex->get_size()};
    _texID = tex->get_id();
    // Replaced glFramebufferTextureLayer with glFramebufferTexture2D
    GLCHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texID, 0));

    // For depth attachment
    GLCHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rbo));
    GLCHECK(glBindRenderbuffer(GL_RENDERBUFFER, _rbo));
    GLCHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size.Width, size.Height));

    // No need for glDrawBuffers, only GL_COLOR_ATTACHMENT0 is supported in OpenGL ES 2.0

    [[maybe_unused]] GLenum const complete {glCheckFramebufferStatus(GL_FRAMEBUFFER)};
    assert(complete == GL_FRAMEBUFFER_COMPLETE);
}

void gl_framebuffer::get_subimage(rect_i const& rect, std::span<u8> pixels, GLenum format) const
{
    bind();
    GLCHECK(glReadPixels(rect.left(), rect.top(), rect.width(), rect.height(), format, GL_UNSIGNED_BYTE, pixels.data()));
}

auto gl_framebuffer::read_pixel(point_i pos) const -> color
{
    bind();
    std::array<u8, 4> data {};
    GLCHECK(glReadPixels(pos.X, pos.Y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data.data()));
    return color {data[0], data[1], data[2], data[3]};
}

void gl_framebuffer::clear(color c) const
{
    bind();
    // Replaced glClearBufferfv and glClearBufferfi with glClearColor and glClear
    vec4 color {c.to_float_array()};
    GLCHECK(glClearColor(color[0], color[1], color[2], color[3]));
    GLCHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}
}
