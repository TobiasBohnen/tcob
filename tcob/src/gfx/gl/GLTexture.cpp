// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/gl/GLTexture.hpp>

#include <cassert>

#include <glad/gl.h>

namespace tcob::gl {
constexpr auto convert_enum(tcob::gl::TextureFormat format) -> std::pair<GLenum, GLenum>
{
    switch (format) {
    case tcob::gl::TextureFormat::R8:
        return { GL_R8, GL_RED };
    case tcob::gl::TextureFormat::RGB8:
        return { GL_RGB8, GL_RGB };
    case tcob::gl::TextureFormat::RGBA8:
        return { GL_RGBA8, GL_RGBA };
    default:
        return { GL_INVALID_ENUM, GL_INVALID_ENUM };
    }
}

constexpr auto convert_enum(tcob::gl::TextureFiltering filtering) -> GLenum
{
    switch (filtering) {
    case tcob::gl::TextureFiltering::Linear:
        return GL_LINEAR;
    case tcob::gl::TextureFiltering::NearestNeighbor:
        return GL_NEAREST;
    default:
        return GL_INVALID_ENUM;
    }
}

constexpr auto convert_enum(tcob::gl::TextureWrap wrap) -> GLenum
{
    switch (wrap) {
    case tcob::gl::TextureWrap::ClampToEdge:
        return GL_CLAMP_TO_EDGE;
    case tcob::gl::TextureWrap::ClampToBorder:
        return GL_CLAMP_TO_BORDER;
    case tcob::gl::TextureWrap::MirroredRepeat:
        return GL_MIRRORED_REPEAT;
    case tcob::gl::TextureWrap::Repeat:
        return GL_REPEAT;
    case tcob::gl::TextureWrap::MirrorClampToEdge:
        return GL_MIRROR_CLAMP_TO_EDGE;
    default:
        return GL_INVALID_ENUM;
    }
}

////////////////////////////////////////////////////////////

Texture::~Texture()
{
    destroy();
}

void Texture::filtering(TextureFiltering filter) const
{
    assert(ID);
    const GLenum filtering = convert_enum(filter);

    glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, filtering);
    glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, filtering);
}

void Texture::wrapping(TextureWrap wrap) const
{
    wrapping(wrap, wrap);
}

void Texture::wrapping(TextureWrap wrapS, TextureWrap wrapT) const
{
    glTextureParameteri(ID, GL_TEXTURE_WRAP_S, convert_enum(wrapS));
    glTextureParameteri(ID, GL_TEXTURE_WRAP_T, convert_enum(wrapT));
}

auto Texture::size() const -> SizeU
{
    return _size;
}

void Texture::size(const SizeU& size)
{
    _size = size;
}

auto Texture::regions() -> std::unordered_map<std::string, TextureRegion>&
{
    return _regions;
}

void Texture::do_destroy()
{
    glDeleteTextures(1, &ID);
}

void Texture::bind_texture_unit(u32 tu) const
{
    assert(ID);
    glBindTextureUnit(tu, ID);
}

auto Texture::copy_to_image() const -> Image
{
    std::vector<ubyte> buffer;
    buffer.reserve(_size.Width * _size.Height * 4);

    glGetTextureImage(ID, 0, GL_RGBA, GL_UNSIGNED_BYTE, static_cast<GLsizei>(buffer.capacity()), buffer.data());

    return Image::CreateFromBuffer(_size, 4, buffer.data());
}

////////////////////////////////////////////////////////////

Texture2D::Texture2D()
{
}

void Texture2D::create(const SizeU& texsize, TextureFormat format)
{
    if (ID) {
        destroy();
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &ID);
    wrapping(TextureWrap::Repeat, TextureWrap::Repeat);
    filtering(TextureFiltering::NearestNeighbor);

    const auto [iform, form] = convert_enum(format);
    _format = form;

    glTextureStorage2D(ID, 1, iform, texsize.Width, texsize.Height);
    size(texsize);
}

void Texture2D::update(const PointU& origin, const SizeU& size, const void* data, i32 rowLength, i32 alignment) const
{
    assert(ID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
    glTextureSubImage2D(ID, 0, origin.X, origin.Y, size.Width, size.Height, _format, GL_UNSIGNED_BYTE, data);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

auto Texture2D::format() -> TextureFormat
{
    switch (_format) {
    case GL_RED:
        return tcob::gl::TextureFormat::R8;
    case GL_RGB:
        return tcob::gl::TextureFormat::RGB8;
    case GL_RGBA:
        return tcob::gl::TextureFormat::RGBA8;
    default:
        return tcob::gl::TextureFormat::R8;
    }
}

////////////////////////////////////////////////////////////

Texture2DArray::Texture2DArray()
{
}

void Texture2DArray::create(const SizeU& texsize, u32 depth, TextureFormat format)
{
    if (ID) {
        destroy();
    }

    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &ID);
    wrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge);
    filtering(TextureFiltering::NearestNeighbor);

    _depth = depth;

    const auto [iform, form] = convert_enum(format);
    _format = form;

    glTextureStorage3D(ID, 1, iform, texsize.Width, texsize.Height, depth);
    size(texsize);
}

void Texture2DArray::update(const PointU& origin, const SizeU& size, const void* data, u32 depth, i32 rowLength, i32 alignment) const
{
    assert(ID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
    glTextureSubImage3D(ID, 0, origin.X, origin.Y, depth, size.Width, size.Height, 1, _format, GL_UNSIGNED_BYTE, data);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

auto Texture2DArray::copy_to_image() const -> Image
{
    std::vector<ubyte> buffer;
    buffer.reserve(size().Width * size().Height * 4 * _depth);

    glGetTextureImage(ID, 0, GL_RGBA, GL_UNSIGNED_BYTE, static_cast<GLsizei>(buffer.capacity()), buffer.data());

    return Image::CreateFromBuffer({ size().Width, size().Height * _depth }, 4, buffer.data());
}

auto Texture2DArray::format() -> TextureFormat
{
    switch (_format) {
    case GL_RED:
        return tcob::gl::TextureFormat::R8;
    case GL_RGB:
        return tcob::gl::TextureFormat::RGB8;
    case GL_RGBA:
        return tcob::gl::TextureFormat::RGBA8;
    default:
        return tcob::gl::TextureFormat::R8;
    }
}

auto Texture2DArray::depth() const -> u32
{
    return _depth;
}

////////////////////////////////////////////////////////////

Texture1D::Texture1D()
{
}

void Texture1D::create(u32 texsize)
{
    if (ID) {
        destroy();
    }

    glCreateTextures(GL_TEXTURE_1D, 1, &ID);
    wrapping(TextureWrap::Repeat, TextureWrap::Repeat);
    filtering(TextureFiltering::Linear);

    glTextureStorage1D(ID, 1, GL_RGBA8, texsize);
    size({ texsize, 1 });
}

void Texture1D::update(i32 offsetX, i32 width, const void* data) const
{
    assert(ID);
    glTextureSubImage1D(ID, 0, offsetX, width, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

auto Texture1D::format() -> TextureFormat
{
    return tcob::gl::TextureFormat::RGBA8;
}
}