// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLES30VertexArray.hpp"

#include <algorithm>
#include <cassert>
#include <span>

#include "GLES30.hpp"
#include "GLES30Enum.hpp"

#include "tcob/core/Logger.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx::gles30 {
gl_vertex_array::gl_vertex_array(buffer_usage_hint usage)
    : _usage {convert_enum(usage)}
{
    GLCHECK(glGenVertexArrays(1, &ID));
    GLCHECK(glGenBuffers(1, &_vbo));
    GLCHECK(glGenBuffers(1, &_ebo));

    setup_attributes();
}

gl_vertex_array::~gl_vertex_array()
{
    destroy();
}

void gl_vertex_array::setup_attributes() const
{
    bind();

    usize offset {0};
    u32   index {0};

    // position attribute
    GLCHECK(glEnableVertexAttribArray(index));
    GLCHECK(glVertexAttribPointer(
        index, sizeof(decltype(vertex::Position)) / sizeof(f32), GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offset))); // NOLINT(performance-no-int-to-ptr)
    offset += sizeof(vertex::Position);
    index++;

    // color attribute
    GLCHECK(glEnableVertexAttribArray(index));
    GLCHECK(glVertexAttribPointer(
        index, sizeof(decltype(vertex::Color)) / sizeof(u8), GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex), reinterpret_cast<void*>(offset))); // NOLINT(performance-no-int-to-ptr)
    offset += sizeof(vertex::Color);
    index++;

    // texture coord attribute
    GLCHECK(glEnableVertexAttribArray(index));
    GLCHECK(glVertexAttribPointer(
        index, sizeof(decltype(vertex::TexCoords)) / sizeof(f32), GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offset))); // NOLINT(performance-no-int-to-ptr)
    offset += sizeof(vertex::TexCoords);
    index++;

    static_assert(sizeof(vertex) == sizeof(vertex::Position) + sizeof(vertex::Color) + sizeof(vertex::TexCoords));
    static_assert(sizeof(vertex) == 24);
    unbind();
}

void gl_vertex_array::do_destroy()
{
    GLCHECK(glDeleteVertexArrays(1, &ID));
    GLCHECK(glDeleteBuffers(1, &_vbo));
    _vbo = 0;
    GLCHECK(glDeleteBuffers(1, &_ebo));
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

        bind();
        GLCHECK(glBufferData(GL_ARRAY_BUFFER, _vboSize, nullptr, _usage));
        GLCHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, _eboSize, nullptr, _usage));
        unbind();
    }
}

void gl_vertex_array::bind() const
{
    assert(ID);
    GLCHECK(glBindVertexArray(ID));
    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
    GLCHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo));
}

void gl_vertex_array::unbind() const
{
    assert(ID);
    GLCHECK(glBindVertexArray(0));
    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void gl_vertex_array::update_data(std::span<vertex const> verts, usize vertOffset) const
{
    bind();
    assert(_vboSize >= verts.size_bytes() + (vertOffset * sizeof(vertex)));
    GLCHECK(glBufferSubData(GL_ARRAY_BUFFER, vertOffset * sizeof(vertex), verts.size_bytes(), verts.data()));
    unbind();
}

void gl_vertex_array::update_data(std::span<quad const> quads, usize quadOffset) const
{
    bind();
    assert(_vboSize >= quads.size_bytes() + (quadOffset * sizeof(quad)));
    GLCHECK(glBufferSubData(GL_ARRAY_BUFFER, quadOffset * sizeof(quad), quads.size_bytes(), quads.data()));
    unbind();
}

void gl_vertex_array::update_data(std::span<u32 const> inds, usize indOffset) const
{
    bind();
    assert(_eboSize >= inds.size_bytes() + (indOffset * sizeof(u32)));
    GLCHECK(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indOffset * sizeof(u32), inds.size_bytes(), inds.data()));
    unbind();
}

void gl_vertex_array::draw_elements(primitive_type mode, usize count, u32 offset) const
{
    bind();
    GLCHECK(glDrawElements(convert_enum(mode), static_cast<GLsizei>(count), GL_UNSIGNED_INT, reinterpret_cast<void*>(offset * sizeof(GLuint)))); // NOLINT(performance-no-int-to-ptr)
    unbind();
}

void gl_vertex_array::draw_arrays(primitive_type mode, i32 first, usize count) const
{
    bind();
    GLCHECK(glDrawArrays(convert_enum(mode), static_cast<GLint>(first), static_cast<GLsizei>(count)));
    unbind();
}

}
