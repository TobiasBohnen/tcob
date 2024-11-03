// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API shader final {
public:
    shader();
    ~shader();

    explicit operator bool() const;

    auto create(string const& vertexShaderSource, string const& fragmentShaderSource) -> bool;

    template <std::derived_from<render_backend::shader_base> T>
    auto get_impl() const -> T*;

    static inline char const* asset_name {"shader"};

private:
    std::unique_ptr<render_backend::shader_base> _impl;
};

}

#include "ShaderProgram.inl"
