// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Tween.hpp"

#include <memory>

#include "tcob/core/Signal.hpp"
#include "tcob/core/easing/Easing.hpp"

namespace tcob {

////////////////////////////////////////////////////////////

template <easing::Function Func>
inline tween<Func>::tween(milliseconds duration)
    : tween_base {duration}
{
}

template <easing::Function Func>
inline tween<Func>::tween(milliseconds duration, func_type&& func)
    : tween_base {duration}
    , Function {std::move(func)}
{
}

template <easing::Function Func>
inline auto tween<Func>::add_output(value_type* dest) -> connection
{
    return Value.Changed.connect([dest](value_type const& val) { *dest = val; });
}

template <easing::Function Func>
inline void tween<Func>::update_values()
{
    Value = Function(progress());
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
