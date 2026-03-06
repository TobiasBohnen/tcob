// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////
// TODO: material -> technique -> pass

class TCOB_API pass final {
public:
    asset_ptr<shader>               Shader {};
    asset_ptr<texture>              Texture {};
    std::shared_ptr<uniform_buffer> UniformBuffer {};

    blend_funcs    BlendFuncs {};
    blend_equation BlendEquation {blend_equation::Add};

    color Color {colors::White};

    stencil_func StencilFunc {stencil_func::Always};
    stencil_op   StencilOp {stencil_op::Keep};
    u8           StencilRef {1};

    auto operator==(pass const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API material final {
public:
    material() = default;

    auto first_pass() -> pass&;
    auto create_pass() -> pass&;

    auto get_pass(isize idx) -> pass&;
    auto get_pass(isize idx) const -> pass const&;
    auto pass_count() const -> isize;

    static inline char const* AssetName {"material"};

    static auto Empty() -> asset_owner_ptr<material>;

    auto operator==(material const& other) const -> bool = default;

private:
    std::vector<pass> _passes;
};

}
