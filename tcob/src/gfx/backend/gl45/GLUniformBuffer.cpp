// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLUniformBuffer.hpp"

#include <cassert>

#include <glad/gl45.h>

#include "tcob/core/Logger.hpp"

namespace tcob::gfx::gl45 {
gl_uniform_buffer::gl_uniform_buffer(usize size)
{
    glCreateBuffers(1, &ID);
    glNamedBufferStorage(ID, size, nullptr, GL_DYNAMIC_STORAGE_BIT);

    logger::Debug("UniformBuffer: created ID {}: size {}", ID, size);
}

gl_uniform_buffer::~gl_uniform_buffer()
{
    destroy();
}

void gl_uniform_buffer::do_destroy()
{
    glDeleteBuffers(1, &ID);
}

void gl_uniform_buffer::bind_base(u32 index) const
{
    assert(ID);
    glBindBufferBase(GL_UNIFORM_BUFFER, index, ID);
}

void gl_uniform_buffer::update(void const* data, usize size, usize offset) const
{
    assert(ID);
    glNamedBufferSubData(ID, offset, size, data);
}
}
