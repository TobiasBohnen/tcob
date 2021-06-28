// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/gl/GLShaderStorageBuffer.hpp>

#include <cassert>

#include <glad/gl.h>

#include <tcob/core/io/Logger.hpp>

namespace tcob::gl {
ShaderStorageBuffer::ShaderStorageBuffer(isize size, BufferUsage usage)
{
    glCreateBuffers(1, &ID);
    resize(size, usage);
}

ShaderStorageBuffer::~ShaderStorageBuffer()
{
    destroy();
}

void ShaderStorageBuffer::do_destroy()
{
    glDeleteBuffers(1, &ID);
}

void ShaderStorageBuffer::bind_base(u32 index) const
{
    assert(ID);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, ID);
}

void ShaderStorageBuffer::update(const void* data, isize size, isize offset) const
{
    assert(ID);
    glNamedBufferSubData(ID, offset, size, data);
}

void ShaderStorageBuffer::resize(isize size, BufferUsage usage)
{
    assert(ID);
    const GLenum bufferUsage { convert_enum(usage) };

    if (size > _bufferSize) {
        _bufferSize = std::max(size, _bufferSize * 2);
        glNamedBufferData(ID, _bufferSize, nullptr, bufferUsage);
        Log(
            "Resizing shader storage buffer " + std::to_string(ID) + " to "
                + std::to_string(_bufferSize) + " bytes.",
            LogLevel::Debug);
    }
}
}