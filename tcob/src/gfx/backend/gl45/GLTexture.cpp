// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLTexture.hpp"

#include <cassert>

#include <glad/gl45.h>

#include "tcob/core/Logger.hpp"

namespace tcob::gfx::gl45 {
auto constexpr convert_enum(texture::format format) -> std::pair<GLenum, GLenum>
{
    switch (format) {
    case texture::format::R8:
        return {GL_R8, GL_RED};
    case texture::format::RGB8:
        return {GL_RGB8, GL_RGB};
    case texture::format::RGBA8:
        return {GL_RGBA8, GL_RGBA};
    }

    return {};
}

auto constexpr convert_enum(texture::filtering filtering) -> GLenum
{
    switch (filtering) {
    case texture::filtering::Linear:
        return GL_LINEAR;
    case texture::filtering::NearestNeighbor:
        return GL_NEAREST;
    }

    return {};
}

auto constexpr convert_enum(texture::wrapping wrap) -> GLenum
{
    switch (wrap) {
    case texture::wrapping::ClampToEdge:
        return GL_CLAMP_TO_EDGE;
    case texture::wrapping::ClampToBorder:
        return GL_CLAMP_TO_BORDER;
    case texture::wrapping::MirroredRepeat:
        return GL_MIRRORED_REPEAT;
    case texture::wrapping::Repeat:
        return GL_REPEAT;
    case texture::wrapping::MirrorClampToEdge:
        return GL_MIRROR_CLAMP_TO_EDGE;
    }

    return {};
}

////////////////////////////////////////////////////////////

gl_texture::~gl_texture()
{
    destroy();
}

auto gl_texture::get_filtering() const -> texture::filtering
{
    assert(ID);

    GLint filtering {0};
    glGetTextureParameterIiv(ID, GL_TEXTURE_MAG_FILTER, &filtering);

    switch (filtering) {
    case GL_LINEAR:
        return texture::filtering::Linear;
    case GL_NEAREST:
        return texture::filtering::NearestNeighbor;
    }

    return texture::filtering::Linear;
}

void gl_texture::set_filtering(texture::filtering val) const
{
    assert(ID);
    GLenum const filtering {convert_enum(val)};

    glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, filtering);
    glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, filtering);
}

auto gl_texture::get_wrapping() const -> texture::wrapping
{
    assert(ID);

    GLint wrapS {0};
    glGetTextureParameterIiv(ID, GL_TEXTURE_WRAP_S, &wrapS);

    auto convertWrap {
        [](GLint wrap) {
            switch (wrap) {
            case GL_CLAMP_TO_EDGE:
                return texture::wrapping::ClampToEdge;
            case GL_CLAMP_TO_BORDER:
                return texture::wrapping::ClampToBorder;
            case GL_MIRRORED_REPEAT:
                return texture::wrapping::MirroredRepeat;
            case GL_REPEAT:
                return texture::wrapping::Repeat;
            case GL_MIRROR_CLAMP_TO_EDGE:
                return texture::wrapping::MirrorClampToEdge;
            }
            return texture::wrapping::Repeat;
        }};

    return convertWrap(wrapS);
}

void gl_texture::set_wrapping(texture::wrapping val) const
{
    glTextureParameteri(ID, GL_TEXTURE_WRAP_S, convert_enum(val));
    glTextureParameteri(ID, GL_TEXTURE_WRAP_T, convert_enum(val));
}

auto gl_texture::copy_to_image(u32 depth) const -> image
{
    std::vector<ubyte> buffer;
    buffer.resize(_size.Width * _size.Height * 4);

    glGetTextureSubImage(ID, 0, 0, 0, depth, _size.Width, _size.Height, 1, GL_RGBA, GL_UNSIGNED_BYTE, static_cast<GLsizei>(buffer.capacity()), buffer.data());

    return image::Create(_size, image::format::RGBA, buffer);
}

void gl_texture::create(size_i texsize, u32 depth, texture::format format)
{
    _size = texsize;
    if (ID) {
        do_destroy();
    }

    create(GL_TEXTURE_2D_ARRAY);

    auto const [iform, _] {convert_enum(format)};
    glTextureStorage3D(ID, 1, iform, texsize.Width, texsize.Height, depth);

    logger::Debug("Texture: created ID {}: width {}, height {}, depth {}", ID, texsize.Width, texsize.Height, depth);
}

void gl_texture::update(point_i origin, size_i size, void const* data, u32 depth, texture::format format, i32 rowLength, i32 alignment) const
{
    assert(ID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
    auto const [_, form] {convert_enum(format)};
    glTextureSubImage3D(ID, 0, origin.X, origin.Y, depth, size.Width, size.Height, 1, form, GL_UNSIGNED_BYTE, data);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

auto gl_texture::is_valid() const -> bool
{
    return ID != 0;
}

auto gl_texture::get_id() const -> u32
{
    return ID;
}

void gl_texture::do_destroy()
{
    glDeleteTextures(1, &ID);
}

void gl_texture::create(i32 type)
{
    glCreateTextures(type, 1, &ID);
    set_filtering(texture::filtering::NearestNeighbor);
    set_wrapping(texture::wrapping::Repeat);
}
}
