// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "VertexTween.hpp"

#include <memory>
#include <span>
#include <vector>

namespace tcob::gfx {

template <VertexTweenFunction... Funcs>
inline vertex_tween<Funcs...>::vertex_tween(milliseconds duration, Funcs&&... ptr)
    : vertex_tween_base {duration}
    , _functions {std::forward<Funcs>(ptr)...}
{
}

template <VertexTweenFunction... Funcs>
inline void vertex_tween<Funcs...>::update_values()
{
    f64 const   p {progress()};
    auto const& src {source_groups()};
    if (src.empty()) { return; }

    std::vector<vertex_tween_group> working {src};

    std::apply([&](auto&&... funcs) { (funcs(p, std::span<vertex_tween_group> {working}), ...); }, _functions);

    copy_to_dest(working);
}

////////////////////////////////////////////////////////////

template <typename... Funcs>
inline auto vertex_tweens::create(u8 id, milliseconds duration, Funcs&&... args) -> std::shared_ptr<vertex_tween<Funcs...>>
{
    if (id == 0) { return nullptr; } // TODO: log error

    auto retValue {std::shared_ptr<vertex_tween<Funcs...>>(new vertex_tween<Funcs...> {duration, std::forward<Funcs>(args)...})};
    _effects[id] = retValue;
    return retValue;
}

}
