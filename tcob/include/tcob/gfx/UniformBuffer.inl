// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "UniformBuffer.hpp"

#include <span>

#include "tcob/gfx/RenderSystemImpl.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

template <POD T>
inline auto uniform_buffer::update(T data, usize offset) const -> usize
{
    _impl->update(&data, sizeof(data), offset);
    return sizeof(data);
}

template <typename T>
inline auto uniform_buffer::update(std::span<T const> data, usize offset) const -> usize
{
    _impl->update(data.data(), data.size_bytes(), offset);
    return data.size_bytes();
}

}
