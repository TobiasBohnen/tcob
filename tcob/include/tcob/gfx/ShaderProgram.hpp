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

class TCOB_API shader final : public non_copyable {
public:
    shader(std::span<char const> vert, std::span<char const> frag);
    ~shader();

    explicit operator bool() const;
    auto     is_valid() const -> bool;

    template <DerivedFrom<render_backend::shader_base> T>
    auto get_impl() const -> T*;

    static inline char const* AssetName {"shader"};

private:
    std::unique_ptr<render_backend::shader_base> _impl;
};

}

#include "ShaderProgram.inl"
