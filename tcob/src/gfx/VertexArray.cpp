// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/VertexArray.hpp"

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/gfx/RenderSystem.hpp"

namespace tcob::gfx {

vertex_array::vertex_array(buffer_usage_hint usage)
    : _impl {locate_service<render_system>().create_vertex_array(usage)}
{
}

void vertex_array::resize(usize vertCount, usize indCount)
{
    _impl->resize(vertCount, indCount);
}

void vertex_array::update_data(std::span<vertex const> verts, usize vertOffset) const
{
    _impl->update_data(verts, vertOffset);
}

void vertex_array::update_data(std::span<quad const> quads, usize quadOffset) const
{
    _impl->update_data(quads, quadOffset);
}

void vertex_array::update_data(std::span<u32 const> inds, usize indOffset) const
{
    _impl->update_data(inds, indOffset);
}

void vertex_array::draw_elements(primitive_type mode, usize count, u32 offset) const
{
    _impl->draw_elements(mode, count, offset);
}

void vertex_array::draw_arrays(primitive_type mode, i32 first, usize count) const
{
    _impl->draw_arrays(mode, first, count);
}

} // namespace tcob
