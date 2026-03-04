// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "UniformBuffer.hpp"

#include <span>

#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

template <POD T>
inline auto uniform_buffer::update(T data, usize offset) const -> usize
{
    update(&data, sizeof(data), offset);
    return sizeof(data);
}

template <POD T>
inline auto uniform_buffer::update(std::span<T const> data, usize offset) const -> usize
{
    update(data.data(), data.size_bytes(), offset);
    return data.size_bytes();
}

template <std::derived_from<render_backend::uniform_buffer_base> T>
inline auto uniform_buffer::get_impl() const -> T*
{
    return static_cast<T*>(_impl.get());
}

}
