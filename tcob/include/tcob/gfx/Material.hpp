// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ShaderProgram.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

struct material {
    assets::asset_ptr<shader>  Shader {};
    assets::asset_ptr<texture> Texture {};

    blend_funcs    BlendFuncs {};
    blend_equation BlendEquation {blend_equation::Add};

    color Color {colors::White};
    f32   PointSize {1};

    static inline char const* asset_name {"material"};
};

inline auto operator==(material const& left, material const& right) -> bool
{
    return (left.Texture == right.Texture)
        && (left.Shader == right.Shader)
        && (left.BlendFuncs == right.BlendFuncs)
        && (left.BlendEquation == right.BlendEquation)
        && (left.Color == right.Color)
        && (left.PointSize == right.PointSize);
}
}
