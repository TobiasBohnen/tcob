// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/gl/GLPixelBuffer.hpp>

#include <cassert>

#include <glad/gl.h>

namespace tcob::gl {
PixelPackBuffer::PixelPackBuffer(isize size)
{
    create(size);
}

void PixelPackBuffer::create(isize size)
{
    assert(!ID);
    glCreateBuffers(1, &ID);
    glNamedBufferData(ID, size, nullptr, GL_STREAM_READ);
}

PixelPackBuffer::~PixelPackBuffer()
{
    destroy();
}

void PixelPackBuffer::do_destroy()
{
    glDeleteBuffers(1, &ID);
}

void PixelPackBuffer::bind() const
{
    assert(ID);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, ID);
}

void PixelPackBuffer::BindDefault()
{
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

auto PixelPackBuffer::map() const -> void*
{
    assert(ID);
    return glMapNamedBuffer(ID, GL_PIXEL_PACK_BUFFER);
}

void PixelPackBuffer::unmap() const
{
    assert(ID);
    auto result { glUnmapNamedBuffer(ID) };
    if (result != GL_TRUE) {
        //TODO handle error
    }
}
}