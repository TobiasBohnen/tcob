// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/FlatMap.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API shader {
public:
    shader();
    virtual ~shader() = default;

    explicit operator bool() const;

    auto create(string const& vertexShaderSource, string const& fragmentShaderSource) -> bool;

    auto get_uniform_block_binding(string const& name) -> u32;

    template <std::derived_from<render_backend::shader_base> T>
    auto get_impl() const -> T*;

    static inline char const* asset_name {"shader"};

private:
    flat_map<string, i32> _uniformBlockBindings {};

    std::unique_ptr<render_backend::shader_base> _impl;
};

}

#include "ShaderProgram.inl"
