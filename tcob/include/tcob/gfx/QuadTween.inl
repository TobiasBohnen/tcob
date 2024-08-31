// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "QuadTween.hpp"

namespace tcob::gfx {

template <typename... Funcs>
inline quad_tween<Funcs...>::quad_tween(milliseconds duration, Funcs&&... ptr)
    : quad_tween_base {duration}
    , _functions {ptr...}
{
}

template <typename... Funcs>
inline void quad_tween<Funcs...>::update_values()
{
    auto const props {get_props()};
    std::apply([&](auto&&... funcs) { (funcs(props), ...); }, _functions);
}

////////////////////////////////////////////////////////////

template <typename... Funcs>
inline auto quad_tweens::create(u8 id, milliseconds duration, Funcs&&... args) -> std::shared_ptr<quad_tween<Funcs...>>
{
    if (id == 0) {
        // TODO: log error
        return nullptr;
    }

    auto retValue {std::shared_ptr<quad_tween<Funcs...>>(new quad_tween<Funcs...> {duration, std::forward<Funcs>(args)...})};
    _effects[id] = retValue;
    return retValue;
}

}
