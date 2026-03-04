// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <span>

#include "tcob/core/Interfaces.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API uniform_buffer final : public non_copyable {
public:
    explicit uniform_buffer(usize size);
    ~uniform_buffer();

    auto update(bool data, usize offset) const -> usize;

    template <POD T>
    auto update(T data, usize offset) const -> usize;

    template <POD T>
    auto update(std::span<T const> data, usize offset) const -> usize;

    template <std::derived_from<render_backend::uniform_buffer_base> T>
    auto get_impl() const -> T*;

private:
    void update(void const* data, usize size, usize offset) const;

    std::unique_ptr<render_backend::uniform_buffer_base> _impl;
};

}

#include "UniformBuffer.inl"
