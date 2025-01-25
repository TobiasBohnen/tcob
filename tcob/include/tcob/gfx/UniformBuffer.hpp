// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>

#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API uniform_buffer final {
public:
    explicit uniform_buffer(usize size);
    ~uniform_buffer();

    auto update(bool data, usize offset) const -> usize;

    template <POD T>
    auto update(T data, usize offset) const -> usize;

    template <typename T>
    auto update(std::span<T const> data, usize offset) const -> usize;

    void bind_base(u32 index) const;

private:
    std::unique_ptr<render_backend::uniform_buffer_base> _impl;
};

}

#include "UniformBuffer.inl"
