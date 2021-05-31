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
    gl::BlendFunc SourceColorBlendFunc { gl::BlendFunc::SrcAlpha };
    gl::BlendFunc DestinationColorBlendFunc { gl::BlendFunc::OneMinusSrcAlpha };
    gl::BlendFunc SourceAlphaBlendFunc { gl::BlendFunc::SrcAlpha };
    gl::BlendFunc DestinationAlphaBlendFunc { gl::BlendFunc::OneMinusSrcAlpha };
    gl::BlendEquation BlendEquation { gl::BlendEquation::Add };
};

inline auto operator==(const Material& left, const Material& right) -> bool
{
    return (left.Texture.object() == right.Texture.object())
        && (left.Shader.object() == right.Shader.object())
        && (left.SourceColorBlendFunc == right.SourceColorBlendFunc)
        && (left.DestinationColorBlendFunc == right.DestinationColorBlendFunc)
        && (left.SourceAlphaBlendFunc == right.SourceAlphaBlendFunc)
        && (left.DestinationAlphaBlendFunc == right.DestinationAlphaBlendFunc)
        && (left.BlendEquation == right.BlendEquation);
}

inline auto operator!=(const Material& left, const Material& right) -> bool
{
    return !(left == right);
}
}