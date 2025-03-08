// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLES20Texture.hpp"

#include <cassert>
#include <utility>
#include <vector>

#include <glad/gles20.h>

#include "GLES20.hpp"
#include "GLES20Framebuffer.hpp"

#include "tcob/core/Logger.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx::gles20 {

auto constexpr convert_enum(texture::format format) -> std::pair<GLenum, GLenum>
{
    switch (format) {
    case texture::format::R8: return {GL_LUMINANCE, GL_LUMINANCE}; // Use LUMINANCE for R8
    case texture::format::RGB8: return {GL_RGB, GL_RGB};
    case texture::format::RGBA8: return {GL_RGBA, GL_RGBA};
    }

    return {};
}

auto constexpr convert_enum(texture::filtering filtering) -> GLenum
{
    switch (filtering) {
    case texture::filtering::Linear: return GL_LINEAR;
    case texture::filtering::NearestNeighbor: return GL_NEAREST;
    }

    return {};
}

auto constexpr convert_enum(texture::wrapping wrap) -> GLenum
{
    switch (wrap) {
    case texture::wrapping::ClampToEdge:
    case texture::wrapping::ClampToBorder:     // TODO: not supported
    case texture::wrapping::MirrorClampToEdge: // TODO: not supported
        return GL_CLAMP_TO_EDGE;
    case texture::wrapping::MirroredRepeat: return GL_MIRRORED_REPEAT;
    case texture::wrapping::Repeat: return GL_REPEAT;
    }

    return {};
}

////////////////////////////////////////////////////////////

gl_texture::~gl_texture()
{
    destroy();
}

void gl_texture::bind() const
{
    assert(ID);
    GLCHECK(glBindTexture(GL_TEXTURE_2D, ID)); // Replaced GL_TEXTURE_2D_ARRAY with GL_TEXTURE_2D
}

auto gl_texture::get_filtering() const -> texture::filtering
{
    bind();

    GLint filtering {0};
    GLCHECK(glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &filtering));

    switch (filtering) {
    case GL_LINEAR: return texture::filtering::Linear;
    case GL_NEAREST: return texture::filtering::NearestNeighbor;
    }

    return texture::filtering::Linear;
}

void gl_texture::set_filtering(texture::filtering val) const
{
    bind();

    GLenum const filtering {convert_enum(val)};

    GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering));
    GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering));
}

auto gl_texture::get_wrapping() const -> texture::wrapping
{
    bind();

    GLint wrapS {0};
    GLCHECK(glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrapS));

    auto const convertWrap {
        [](GLint wrap) {
            switch (wrap) {
            case GL_CLAMP_TO_EDGE: return texture::wrapping::ClampToEdge;
            case GL_MIRRORED_REPEAT: return texture::wrapping::MirroredRepeat;
            case GL_REPEAT: return texture::wrapping::Repeat;
            }
            return texture::wrapping::Repeat;
        }};

    return convertWrap(wrapS);
}

void gl_texture::set_wrapping(texture::wrapping val) const
{
    bind();
    GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, convert_enum(val)));
    GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, convert_enum(val)));
}

auto gl_texture::copy_to_image(u32 depth) const -> image
{
    bind();

    std::vector<ubyte> buffer(static_cast<usize>(_size.Width * _size.Height * 4));

    gl_framebuffer frameBuffer;
    frameBuffer.attach_texture(this, depth);

    frameBuffer.get_subimage({point_i::Zero, _size}, buffer, GL_RGBA);

    return image::Create(_size, image::format::RGBA, buffer);
}

void gl_texture::create(size_i texsize, u32 /* depth */, texture::format format)
{
    _size   = texsize;
    _format = format;

    if (ID) {
        do_destroy();
    }

    create();
    bind();

    auto const [iform, _] {convert_enum(format)};
    // Replaced glTexStorage3D with glTexImage2D for 2D textures
    GLCHECK(glTexImage2D(GL_TEXTURE_2D, 0, iform, texsize.Width, texsize.Height, 0, iform, GL_UNSIGNED_BYTE, nullptr));

    logger::Debug("Texture: created ID {}: width {}, height {}", ID, texsize.Width, texsize.Height);
}

void gl_texture::update(point_i origin, size_i size, void const* data, u32 /* depth */, i32 /* rowLength */, i32 alignment) const
{
    bind();
    GLCHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, alignment));
    auto const [_, form] {convert_enum(_format)};
    GLCHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, origin.X, origin.Y, size.Width, size.Height, form, GL_UNSIGNED_BYTE, data));
    GLCHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
}

auto gl_texture::is_valid() const -> bool
{
    return ID != 0;
}

auto gl_texture::get_size() const -> size_i
{
    return _size;
}

void gl_texture::do_destroy()
{
    GLCHECK(glDeleteTextures(1, &ID));
}

void gl_texture::create()
{
    GLCHECK(glGenTextures(1, &ID));
    set_filtering(texture::filtering::NearestNeighbor);
    set_wrapping(texture::wrapping::Repeat);
}
}
