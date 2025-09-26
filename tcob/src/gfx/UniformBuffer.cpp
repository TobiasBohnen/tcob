// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/UniformBuffer.hpp"

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/gfx/RenderSystem.hpp"

namespace tcob::gfx {

uniform_buffer::uniform_buffer(usize size)
    : _impl {locate_service<render_system>().create_uniform_buffer(size)}
{
}

uniform_buffer::~uniform_buffer() = default;

auto uniform_buffer::update(bool data, usize offset) const -> usize
{
    u32 const d {data}; // bools are u32 in GLSL and HLSL
    _impl->update(&d, sizeof(d), offset);
    return sizeof(d);
}

void uniform_buffer::bind_base(u32 index) const
{
    _impl->bind_base(index);
}

}
