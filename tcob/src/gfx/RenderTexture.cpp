// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/RenderTexture.hpp"

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx {

render_texture::render_texture()
    : render_target {this}
{
    add_region("default", {GetTexcoords(), 0});
}

auto render_texture::GetTexcoords() -> rect_f
{
    return locate_service<render_system>().get_rtt_coords();
}

auto render_texture::get_size() const -> size_i
{
    return texture::get_size();
}

}
