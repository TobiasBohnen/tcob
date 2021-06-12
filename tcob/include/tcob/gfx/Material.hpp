// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/assets/Resource.hpp>
#include <tcob/core/data/Rect.hpp>
#include <tcob/gfx/gl/GLEnum.hpp>

namespace tcob {

struct Material final {
    ResourcePtr<gl::ShaderProgram> Shader;
    ResourcePtr<gl::Texture> Texture;
    gl::BlendFuncs BlendFuncs {};
    gl::BlendEquation BlendEquation { gl::BlendEquation::Add };
};

inline auto operator==(const Material& left, const Material& right) -> bool
{
    return (left.Texture.object() == right.Texture.object())
        && (left.Shader.object() == right.Shader.object())
        && (left.BlendFuncs == right.BlendFuncs);
}

inline auto operator!=(const Material& left, const Material& right) -> bool
{
    return !(left == right);
}
}