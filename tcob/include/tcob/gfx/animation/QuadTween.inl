// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "QuadTween.hpp"

namespace tcob::gfx {

template <QuadTweenFunction... Funcs>
inline quad_tween<Funcs...>::quad_tween(milliseconds duration, Funcs&&... ptr)
    : quad_tween_base {duration}
    , _functions {ptr...}
{
}

template <QuadTweenFunction... Funcs>
inline void quad_tween<Funcs...>::update_values()
{
    f64 const p {progress()};

    // copy original quads
    auto source {get_source_quads()};
    if (source.empty()) { return; }

    // run functions
    std::apply([&](auto&&... funcs) { (funcs(p, source), ...); }, _functions);

    // set target
    set_quads(source);
}

////////////////////////////////////////////////////////////

template <typename... Funcs>
inline auto quad_tweens::create(u8 id, milliseconds duration, Funcs&&... args) -> std::shared_ptr<quad_tween<Funcs...>>
{
    if (id == 0) { return nullptr; } // TODO: log error

    auto retValue {std::shared_ptr<quad_tween<Funcs...>>(new quad_tween<Funcs...> {duration, std::forward<Funcs>(args)...})};
    _effects[id] = retValue;
    return retValue;
}

}
