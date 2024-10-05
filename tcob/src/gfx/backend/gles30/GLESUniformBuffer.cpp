// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLESUniformBuffer.hpp"

#include <cassert>

#include <glad/gles30.h>

#include "GLES30.hpp"

namespace tcob::gfx::gles30 {
gl_uniform_buffer::gl_uniform_buffer(usize size)
{
    GLCHECK(glGenBuffers(1, &ID));
    GLCHECK(glBindBuffer(GL_UNIFORM_BUFFER, ID));
    GLCHECK(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW));

    logger::Debug("UniformBuffer: created ID {}: size {}", ID, size);
}

gl_uniform_buffer::~gl_uniform_buffer()
{
    destroy();
}

void gl_uniform_buffer::do_destroy()
{
    GLCHECK(glDeleteBuffers(1, &ID));
}

void gl_uniform_buffer::bind_base(u32 index) const
{
    assert(ID);
    GLCHECK(glBindBufferBase(GL_UNIFORM_BUFFER, index, ID));
}

void gl_uniform_buffer::update(void const* data, usize size, usize offset) const
{
    assert(ID);
    GLCHECK(glBindBuffer(GL_UNIFORM_BUFFER, ID));
    GLCHECK(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data));
}
}
