// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/gl/GLCapabilities.hpp>

#include <glad/gl.h>
namespace tcob::gl::Capabilities {

void init()
{
    std::array<f32, 2> val0 {};
    glGetFloatv(GL_POINT_SIZE_RANGE, val0.data());
    PointSizeRange = { val0[0], val0[1] };

    glGetFloatv(GL_POINT_SIZE_GRANULARITY, &PointSizeGranularity);

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MaxTextureSize);

    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &MaxArrayTextureLayers);
}

}