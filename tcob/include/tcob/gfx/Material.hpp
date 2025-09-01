// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Color.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ShaderProgram.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API material {
public:
    asset_ptr<shader>  Shader {};
    asset_ptr<texture> Texture {};

    blend_funcs    BlendFuncs {};
    blend_equation BlendEquation {blend_equation::Add};

    color Color {colors::White};
    f32   PointSize {1};

    stencil_func StencilFunc {stencil_func::Always};
    stencil_op   StencilOp {stencil_op::Keep};
    u8           StencilRef {1};

    static inline char const* AssetName {"material"};

    static auto Empty() -> asset_owner_ptr<material>;

    auto operator==(material const& other) const -> bool = default;
};

}
