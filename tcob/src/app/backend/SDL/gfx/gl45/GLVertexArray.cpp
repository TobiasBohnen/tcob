// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLVertexArray.hpp"

#include <algorithm>
#include <cassert>
#include <span>

#include "GLEnum.hpp"

#include "tcob/core/Logger.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx::gl45 {

gl_vertex_array::gl_vertex_array(buffer_usage_hint usage)
    : _usage {convert_enum(usage)}
{
    glCreateVertexArrays(1, &ID);
    setup_attributes();

    glCreateBuffers(1, &_vbo);
    glCreateBuffers(1, &_ebo);

    glVertexArrayVertexBuffer(ID, 0, _vbo, 0, sizeof(vertex));
    glVertexArrayElementBuffer(ID, _ebo);
}

gl_vertex_array::~gl_vertex_array()
{
    destroy();
}

void gl_vertex_array::setup_attributes() const
{
    assert(ID);
    u32 offset {0};
    u32 index {0};

    // position attribute
    glVertexArrayAttribBinding(ID, index, 0);
    glVertexArrayAttribFormat(ID, index, sizeof(decltype(vertex::Position)) / sizeof(f32), GL_FLOAT, GL_FALSE, offset);
    glEnableVertexArrayAttrib(ID, index);
    offset += sizeof(vertex::Position);
    index++;

    // color attribute
    glVertexArrayAttribBinding(ID, index, 0);
    glVertexArrayAttribFormat(ID, index, sizeof(decltype(vertex::Color)) / sizeof(u8), GL_UNSIGNED_BYTE, GL_TRUE, offset);
    glEnableVertexArrayAttrib(ID, index);
    offset += sizeof(vertex::Color);
    index++;

    // texture coord attribute
    glVertexArrayAttribBinding(ID, index, 0);
    glVertexArrayAttribFormat(ID, index, sizeof(decltype(vertex::TexCoords)) / sizeof(f32), GL_FLOAT, GL_FALSE, offset);
    glEnableVertexArrayAttrib(ID, index);
    offset += sizeof(vertex::TexCoords);
    index++;

    static_assert(sizeof(vertex) == sizeof(vertex::Position) + sizeof(vertex::Color) + sizeof(vertex::TexCoords));
    static_assert(sizeof(vertex) == 24);
}

void gl_vertex_array::do_destroy()
{
    glDeleteVertexArrays(1, &ID);
    glDeleteBuffers(1, &_vbo);
    _vbo = 0;
    glDeleteBuffers(1, &_ebo);
    _ebo = 0;
}

void gl_vertex_array::resize(usize vertCount, usize indCount)
{
    assert(ID);

    usize const newVboSize {vertCount * sizeof(vertex)};
    usize const newEboSize {indCount * sizeof(GLuint)};
    if (newVboSize > _vboSize || newEboSize > _eboSize) {
        bool creating {_vboSize == 0};
        _vboSize = std::max(newVboSize, _vboSize * 2);
        _eboSize = std::max(newEboSize, _eboSize * 2);

        if (creating) {
            logger::Debug("VertexArray: created ID {}: {} vertices, {} indices",
                          ID, _vboSize / sizeof(vertex), _eboSize / sizeof(GLuint));

        } else {
            logger::Debug("VertexArray: resized ID {}: {} vertices, {} indices",
                          ID, _vboSize / sizeof(vertex), _eboSize / sizeof(GLuint));
        }

        glNamedBufferData(_vbo, _vboSize, nullptr, _usage);
        glNamedBufferData(_ebo, _eboSize, nullptr, _usage);
    }
}

void gl_vertex_array::update_data(std::span<vertex const> verts, usize vertOffset) const
{
    assert(ID);
    assert(_vboSize >= verts.size_bytes() + (vertOffset * sizeof(vertex)));
    glNamedBufferSubData(_vbo, vertOffset * sizeof(vertex), verts.size_bytes(), verts.data());
}

void gl_vertex_array::update_data(std::span<quad const> quads, usize quadOffset) const
{
    assert(ID);
    assert(_vboSize >= quads.size_bytes() + (quadOffset * sizeof(quad)));
    glNamedBufferSubData(_vbo, quadOffset * sizeof(quad), quads.size_bytes(), quads.data());
}

void gl_vertex_array::update_data(std::span<u32 const> inds, usize indOffset) const
{
    assert(ID);
    assert(_eboSize >= inds.size_bytes() + (indOffset * sizeof(GLuint)));
    glNamedBufferSubData(_ebo, indOffset * sizeof(GLuint), inds.size_bytes(), inds.data());
}

void gl_vertex_array::draw_elements(primitive_type mode, usize count, u32 offset) const
{
    assert(ID);
    glBindVertexArray(ID);
    glDrawElements(convert_enum(mode), static_cast<GLsizei>(count), GL_UNSIGNED_INT, reinterpret_cast<void*>(offset * sizeof(GLuint))); // NOLINT(performance-no-int-to-ptr)
    glBindVertexArray(0);
}

void gl_vertex_array::draw_arrays(primitive_type mode, i32 first, usize count) const
{
    assert(ID);
    glBindVertexArray(ID);
    glDrawArrays(convert_enum(mode), static_cast<GLint>(first), static_cast<GLsizei>(count));
    glBindVertexArray(0);
}

}
