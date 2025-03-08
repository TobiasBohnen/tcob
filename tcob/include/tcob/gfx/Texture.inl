// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Texture.hpp"

#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {

template <std::derived_from<render_backend::texture_base> T>
inline auto texture::get_impl() const -> T*
{
    return static_cast<T*>(_impl.get());
}

}
