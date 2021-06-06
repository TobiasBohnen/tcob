// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/gl/GLUniformBuffer.hpp>

#include <cassert>

#include <glad/gl.h>

namespace tcob::gl {
UniformBuffer::UniformBuffer(isize size)
{
    glCreateBuffers(1, &ID);
    glNamedBufferStorage(ID, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
}

UniformBuffer::~UniformBuffer()
{
    destroy();
}

void UniformBuffer::do_destroy()
{
    glDeleteBuffers(1, &ID);
}

void UniformBuffer::bind_base(u32 index) const
{
    assert(ID);
    glBindBufferBase(GL_UNIFORM_BUFFER, index, ID);
}

void UniformBuffer::update(const void* data, isize size, isize offset) const
{
    assert(ID);
    glNamedBufferSubData(ID, offset, size, data);
}
}