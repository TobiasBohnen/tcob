// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/gl/GLRenderTexture.hpp>

#include <tcob/gfx/gl/GLTexture.hpp>

namespace tcob::gl {
void RenderTexture::create(const SizeU& size)
{
    setup_framebuffer(size);
    texture()->regions()["default"] = TextureRegion { { 0, 0, 1, -1 } };
}

auto RenderTexture::size() const -> SizeU
{
    auto tex { texture() };
    return tex ? tex->size() : SizeU::Zero;
}

}