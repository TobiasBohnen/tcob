// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "ShaderProgram.hpp"

namespace tcob::gfx {

template <std::derived_from<render_backend::shader_base> T>
inline auto shader::get_impl() const -> T*
{
    return static_cast<T*>(_impl.get());
}

}
