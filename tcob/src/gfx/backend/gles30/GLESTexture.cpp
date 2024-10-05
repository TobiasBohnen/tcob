// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLESTexture.hpp"

#include <cassert>

#include <glad/gles30.h>

#include "GLES30.hpp"
#include "GLESFramebuffer.hpp"

namespace tcob::gfx::gles30 {
auto constexpr convert_enum(texture::format format) -> std::pair<GLenum, GLenum>
{
    switch (format) {
    case texture::format::R8: return {GL_R8, GL_RED};
    case texture::format::RGB8: return {GL_RGB8, GL_RGB};
    case texture::format::RGBA8: return {GL_RGBA8, GL_RGBA};
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
    GLCHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, ID));
}

auto gl_texture::get_filtering() const -> texture::filtering
{
    bind();

    GLint filtering {0};
    GLCHECK(glGetTexParameteriv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, &filtering));

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

    GLCHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, filtering));
    GLCHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, filtering));
}

auto gl_texture::get_wrapping() const -> texture::wrapping
{
    bind();

    GLint wrapS {0};
    GLCHECK(glGetTexParameteriv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, &wrapS));

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
    GLCHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, convert_enum(val)));
    GLCHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, convert_enum(val)));
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

void gl_texture::create(size_i texsize, u32 depth, texture::format format)
{
    _size = texsize;

    if (ID) {
        do_destroy();
    }

    create();
    bind();

    auto const [iform, _] {convert_enum(format)};
    GLCHECK(glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, iform, texsize.Width, texsize.Height, depth));

    logger::Debug("Texture: created ID {}: width {}, height {}, depth {}", ID, texsize.Width, texsize.Height, depth);
}

void gl_texture::update(point_i origin, size_i size, void const* data, u32 depth, texture::format format, i32 rowLength, i32 alignment) const
{
    bind();
    GLCHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, alignment));
    GLCHECK(glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength));
    auto const [_, form] {convert_enum(format)};
    GLCHECK(glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, origin.X, origin.Y, depth, size.Width, size.Height, 1, form, GL_UNSIGNED_BYTE, data));
    GLCHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
    GLCHECK(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
}

auto gl_texture::is_valid() const -> bool
{
    return ID != 0;
}

auto gl_texture::get_id() const -> u32
{
    return ID;
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
