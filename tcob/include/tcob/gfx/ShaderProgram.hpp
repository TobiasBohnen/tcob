// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>

#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API shader final {
public:
    shader(string const& vertexShaderSource, string const& fragmentShaderSource);
    ~shader();

    explicit operator bool() const;
    auto     is_valid() const -> bool;

    template <std::derived_from<render_backend::shader_base> T>
    auto get_impl() const -> T*;

    static inline char const* AssetName {"shader"};

private:
    auto compile(string const& vertexShaderSource, string const& fragmentShaderSource) -> bool;

    std::unique_ptr<render_backend::shader_base> _impl;
};

}

#include "ShaderProgram.inl"
