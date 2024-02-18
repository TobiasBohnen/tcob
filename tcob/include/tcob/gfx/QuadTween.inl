// Copyright (c) 2023 Tobias Bohnen
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
    quad_tween_properties props {
        .Progress  = get_progress(),
        .SrcQuads  = get_source_quads(),
        .DestQuads = get_destination_quads()};

    std::apply([&](auto&&... funcs) { (funcs(props), ...); }, _functions);
}

////////////////////////////////////////////////////////////

template <typename... Funcs>
auto make_unique_quad_tween(milliseconds duration, Funcs&&... args) -> std::unique_ptr<quad_tween<Funcs...>>
{
    return std::unique_ptr<quad_tween<Funcs...>>(new quad_tween<Funcs...> {duration, std::forward<Funcs>(args)...});
}

template <typename... Funcs>
auto make_shared_quad_tween(milliseconds duration, Funcs&&... args) -> std::shared_ptr<quad_tween<Funcs...>>
{
    return std::shared_ptr<quad_tween<Funcs...>>(new quad_tween<Funcs...> {duration, std::forward<Funcs>(args)...});
}

}
