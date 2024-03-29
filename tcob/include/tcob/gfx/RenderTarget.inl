// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "RenderTarget.hpp"

namespace tcob::gfx {

template <std::derived_from<render_backend::render_target_base> T>
inline auto render_target::get_impl() const -> T*
{
    return static_cast<T*>(_impl.get());
}

}
