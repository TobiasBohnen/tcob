// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/gfx/gl/GLEnum.hpp>
#include <tcob/gfx/gl/GLObject.hpp>

namespace tcob::gl {
class VertexArray final : public Object {
public:
    VertexArray() = default;
    ~VertexArray() override;

    VertexArray(VertexArray&& other) noexcept = default;
    auto operator=(VertexArray&& other) noexcept -> VertexArray& = default;

    void create_or_resize(isize vertCount, isize indCount, BufferUsage usage);

    void update(const Vertex* verts, isize vertCount, isize vertOffset) const;
    void update(const Quad* quad, isize quadCount, isize vertOffset) const;
    void update(const u32* ind, isize indCount, isize indOffset) const;

    void draw_elements(i32 mode, isize count, u32 offset) const;
    void draw_elements_instanced(i32 mode, isize count, u32 offset, i32 instanceCount) const;
    void draw_arrays(i32 mode, i32 first, isize count) const;

    auto map_vertexbuffer() const -> void*;
    void unmap_vertexbuffer() const;

protected:
    void do_destroy() override;

private:
    void setup_attributes() const;

    u32 _vbo { 0 };
    u32 _ebo { 0 };
    isize _vboSize { 0 };
    isize _eboSize { 0 };
};
}