// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLES30Framebuffer.hpp"

#include <array>
#include <cassert>
#include <span>

#include "GLES30.hpp"
#include "GLES30Texture.hpp"

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx::gles30 {
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

void gl_framebuffer::attach_texture(gl_texture const* tex, u32 depth)
{
    bind();

    auto const size {tex->get_size()};
    _texID = tex->ID;
    GLCHECK(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _texID, 0, depth));

    GLCHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo));
    GLCHECK(glBindRenderbuffer(GL_RENDERBUFFER, _rbo));
    GLCHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.Width, size.Height));

    // Set the list of draw buffers.
    std::array<GLenum, 1> const buffers {GL_COLOR_ATTACHMENT0};
    GLCHECK(glDrawBuffers(static_cast<GLsizei>(buffers.size()), buffers.data()));

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
    GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, ID));
    vec4 color {c.to_float_array()};
    GLCHECK(glClearBufferfv(GL_COLOR, 0, color.data()));
    GLCHECK(glClearBufferfi(GL_DEPTH_STENCIL, 0, 1, 0));
}
}
