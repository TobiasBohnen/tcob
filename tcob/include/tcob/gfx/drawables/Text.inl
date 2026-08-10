// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Text.hpp"

#include <memory>

#include "tcob/gfx/animation/VertexTween.hpp"

namespace tcob::gfx {

template <typename... Funcs>
inline auto text::create_effect(u8 id, milliseconds duration, Funcs&&... args) -> std::shared_ptr<vertex_tween<Funcs...>>
{
    auto retValue {std::shared_ptr<vertex_tween<Funcs...>>(new vertex_tween<Funcs...> {duration, std::forward<Funcs>(args)...})};
    _effects[id] = retValue;
    return retValue;
}

}
