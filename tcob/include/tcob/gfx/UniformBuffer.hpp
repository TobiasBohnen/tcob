// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>

#include "tcob/gfx/RenderSystemImpl.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API uniform_buffer {
public:
    explicit uniform_buffer(usize size);

    auto update(bool data, usize offset) const -> usize;

    template <POD T>
    auto update(T data, usize offset) const -> usize;

    template <typename T>
    auto update(std::span<T const> data, usize offset) const -> usize;

    void bind_base(u32 index) const;

private:
    std::unique_ptr<render_backend::uniform_buffer_base> _impl;
};

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
