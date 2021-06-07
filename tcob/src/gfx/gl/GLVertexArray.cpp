// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/gl/GLVertexArray.hpp>

#include <cassert>

#include <glad/gl.h>

#include <tcob/core/io/Logger.hpp>
#include <tcob/gfx/Quad.hpp>

namespace tcob::gl {
VertexArray::VertexArray()
{
    glCreateVertexArrays(1, &ID);
    setup_attributes();

    glCreateBuffers(1, &_vbo);
    glCreateBuffers(1, &_ebo);

    glVertexArrayVertexBuffer(ID, 0, _vbo, 0, sizeof(Vertex));
    glVertexArrayElementBuffer(ID, _ebo);
}

VertexArray::~VertexArray()
{
    destroy();
}

void VertexArray::setup_attributes() const
{
    assert(ID);
    u32 offset { 0 };
    u32 index { 0 };

    // position attribute
    glVertexArrayAttribBinding(ID, index, 0);
    glVertexArrayAttribFormat(ID, index, std::tuple_size<decltype(Vertex::Position)>::value, GL_FLOAT, GL_FALSE, offset);
    glEnableVertexArrayAttrib(ID, index);
    offset += sizeof(Vertex::Position);
    index++;

    // color attribute
    glVertexArrayAttribBinding(ID, index, 0);
    glVertexArrayAttribFormat(ID, index, std::tuple_size<decltype(Vertex::Color)>::value, GL_UNSIGNED_BYTE, GL_TRUE, offset);
    glEnableVertexArrayAttrib(ID, index);
    offset += sizeof(Vertex::Color);
    index++;

    // texture coord attribute
    glVertexArrayAttribBinding(ID, index, 0);
    glVertexArrayAttribFormat(ID, index, std::tuple_size<decltype(Vertex::TexCoords)>::value, GL_FLOAT, GL_FALSE, offset);
    glEnableVertexArrayAttrib(ID, index);
    offset += sizeof(Vertex::TexCoords);
    index++;

    static_assert(sizeof(Vertex) == sizeof(Vertex::Position) + sizeof(Vertex::Color) + sizeof(Vertex::TexCoords));
}

void VertexArray::do_destroy()
{
    glDeleteVertexArrays(1, &ID);
    glDeleteBuffers(1, &_vbo);
    _vbo = 0;
    glDeleteBuffers(1, &_ebo);
    _ebo = 0;
}

void VertexArray::resize(isize vertCount, isize indCount, BufferUsage usage)
{
    assert(ID);
    const GLenum bufferUsage { enumToGL(usage) };

    const isize newVboSize { vertCount * sizeof(Vertex) };
    const isize newEboSize { indCount * sizeof(GLuint) };
    if (newVboSize > _vboSize || newEboSize > _eboSize) {
        _vboSize = std::max(newVboSize, _vboSize * 2);
        _eboSize = std::max(newEboSize, _eboSize * 2);
        glNamedBufferData(_vbo, _vboSize, nullptr, bufferUsage);
        glNamedBufferData(_ebo, _eboSize, nullptr, bufferUsage);
        Log("Resizing vertex array " + std::to_string(ID) + " to "
                + std::to_string(_vboSize / sizeof(Vertex)) + " vertices and " + std::to_string(_eboSize / sizeof(GLuint)) + " indices.",
            LogLevel::Debug);
    }
}

void VertexArray::update(const Quad* quad, isize quadCount, isize vertOffset) const
{
    assert(ID);
    glNamedBufferSubData(_vbo, vertOffset * sizeof(Vertex), quadCount * sizeof(Quad), quad);
}

void VertexArray::update(const Vertex* verts, isize vertCount, isize vertOffset) const
{
    assert(ID);
    glNamedBufferSubData(_vbo, vertOffset * sizeof(Vertex), vertCount * sizeof(Vertex), verts);
}

void VertexArray::update(const u32* ind, isize indCount, isize indOffset) const
{
    assert(ID);
    glNamedBufferSubData(_ebo, indOffset * sizeof(u32), indCount * sizeof(u32), ind);
}

void VertexArray::draw_elements(i32 mode, isize count, u32 offset) const
{
    assert(ID);
    glBindVertexArray(ID);
    glDrawElements(mode, static_cast<i32>(count), GL_UNSIGNED_INT, reinterpret_cast<void*>(offset * sizeof(GLuint)));
    glBindVertexArray(0);
}

void VertexArray::draw_elements_instanced(i32 mode, isize count, u32 offset, i32 instanceCount) const
{
    assert(ID);
    glBindVertexArray(ID);
    glDrawElementsInstanced(mode, static_cast<i32>(count), GL_UNSIGNED_INT, reinterpret_cast<void*>(offset * sizeof(GLuint)), instanceCount);
    glBindVertexArray(0);
}

void VertexArray::draw_arrays(i32 mode, i32 first, isize count) const
{
    assert(ID);
    glBindVertexArray(ID);
    glDrawArrays(mode, first, static_cast<i32>(count));
    glBindVertexArray(0);
}

auto VertexArray::map_vertexbuffer() const -> void*
{
    assert(ID);
    return glMapNamedBuffer(_vbo, GL_WRITE_ONLY);
}

void VertexArray::unmap_vertexbuffer() const
{
    assert(ID);
    auto result { glUnmapNamedBuffer(_vbo) };
    if (result != GL_TRUE) {
        //TODO handle error
    }
}
}