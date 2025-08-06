// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/RenderTexture.hpp"

#include "tcob/core/Rect.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx {

render_texture::render_texture()
    : render_target {this}
{
    regions()["default"] = {.UVRect = UVRect(), .Level = 0};
}

auto render_texture::UVRect() -> rect_f
{
    return locate_service<render_system>().capabilities().RenderTextureUVRect;
}

auto render_texture::get_size() const -> size_i
{
    return info().Size;
}

}
