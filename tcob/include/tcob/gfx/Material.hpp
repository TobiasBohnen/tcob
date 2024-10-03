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

class TCOB_API material {
public:
    assets::asset_ptr<shader>  Shader {};
    assets::asset_ptr<texture> Texture {};

    blend_funcs    BlendFuncs {};
    blend_equation BlendEquation {blend_equation::Add};

    color Color {colors::White};
    f32   PointSize {1};

    static inline char const* asset_name {"material"};

    auto static Empty() -> assets::manual_asset_ptr<material>;

    auto operator==(material const& other) const -> bool = default;
};

}
