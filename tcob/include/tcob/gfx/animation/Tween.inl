// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Tween.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

template <TweenFunction Func>
inline tween<Func>::tween(milliseconds duration)
    : tween_base {duration}
{
}

template <TweenFunction Func>
inline tween<Func>::tween(milliseconds duration, func_type&& func)
    : tween_base {duration}
    , Function {std::move(func)}
{
}

template <TweenFunction Func>
inline auto tween<Func>::add_output(value_type* dest) -> connection
{
    return Value.Changed.connect([dest](value_type const& val) { *dest = val; });
}

template <TweenFunction Func>
inline void tween<Func>::update_values()
{
    Value = Function(get_progress());
}

////////////////////////////////////////////////////////////

template <typename Tween>
auto make_unique_tween(milliseconds duration, auto&&... args) -> std::unique_ptr<Tween>
{
    return std::unique_ptr<Tween>(new Tween {duration, typename Tween::func_type {args...}});
}

template <typename Tween>
auto make_shared_tween(milliseconds duration, auto&&... args) -> std::shared_ptr<Tween>
{
    return std::shared_ptr<Tween>(new Tween {duration, typename Tween::func_type {args...}});
}

////////////////////////////////////////////////////////////

inline void tween_queue::push(auto&&... autom)
{
    (_queue.push(autom), ...);
}

////////////////////////////////////////////////////////////
}
