// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Closure.hpp"

#include <functional>
#include <tuple>
#include <type_traits>

#include "tcob/scripting/Lua.hpp"

namespace tcob::scripting::detail {

template <typename R, typename... Args>
inline native_closure<R(Args...)>::native_closure(std::function<R(Args...)> fn)
    : _fn {std::move(fn)}
{
}

template <typename R, typename... Args>
inline auto native_closure<R(Args...)>::operator()(state_view view) -> i32
{
    std::tuple<std::remove_cvref_t<Args>...> params;
    std::apply(
        [&view](auto&&... item) {
            [[maybe_unused]] i32 idx {1};
            (view.pull_convert(idx, item), ...);
        },
        params);

    i32 const oldTop {view.get_top()};
    if constexpr (std::is_void_v<R>) {
        std::apply(_fn, params);
    } else {
        view.push_convert(std::apply(_fn, params));
    }
    return view.get_top() - oldTop;
}

////////////////////////////////////////////////////////////

template <typename Arg>
inline auto compare_types_impl(state_view view, i32 startIndex) -> bool
{
    return base_converter<Arg>::IsType(view, startIndex);
}

template <typename R, typename... Args>
inline auto compare_types([[maybe_unused]] state_view view, [[maybe_unused]] i32 startIndex, std::function<R(Args...)> const&) -> bool
{
    return ((compare_types_impl<Args>(view, startIndex++)) && ...);
}

////////////////////////////////////////////////////////////

template <typename... Funcs>
inline native_overload<Funcs...>::native_overload(std::tuple<Funcs...> fns)
    : _fns {std::move(fns)}
{
}

template <typename... Funcs>
inline auto native_overload<Funcs...>::operator()(state_view view) -> i32
{
    i32 const oldTop {view.get_top()};
    if (!std::apply([&](auto&&... item) { return ((try_call(view, oldTop, item)) || ...); }, _fns)) {
        view.error("no matching overload found");
    }
    return view.get_top() - oldTop;
}

template <typename... Funcs>
template <typename R, typename T, typename... Args>
inline auto native_overload<Funcs...>::try_call(state_view view, i32 top, R (T::*func)(Args...)) -> bool
{
    return try_call_func(view, top, std::function<R(T*, Args...)> {func});
}

template <typename... Funcs>
inline auto native_overload<Funcs...>::try_call(state_view view, i32 top, auto&& func) -> bool
{
    return try_call_func(view, top, std::function {func});
}

template <typename... Funcs>
template <typename R, typename... Args>
inline auto native_overload<Funcs...>::try_call_func(state_view view, i32 top, std::function<R(Args...)> const& func) -> bool
{
    if (top == sizeof...(Args) && compare_types(view, 1, func)) {
        std::tuple<std::remove_cvref_t<Args>...> params;
        std::apply(
            [&view](auto&&... item) {
                [[maybe_unused]] i32 idx {1};
                (view.pull_convert(idx, item), ...);
            },
            params);

        if constexpr (std::is_void_v<R>) {
            std::apply(func, params);
        } else {
            view.push_convert(std::apply(func, params));
        }
        return true;
    }

    return false;
}

}

////////////////////////////////////////////////////////////

namespace tcob::scripting {

template <typename R, typename... P>
auto make_unique_closure(std::function<R(P...)>&& fn) -> native_closure_unique_ptr
{
    return std::make_unique<detail::native_closure<R(P...)>>(std::move(fn));
}

template <typename R, typename... P>
auto make_shared_closure(std::function<R(P...)>&& fn) -> native_closure_shared_ptr
{
    return std::make_shared<detail::native_closure<R(P...)>>(std::move(fn));
}

template <typename... Funcs>
auto make_unique_overload(Funcs&&... fns) -> native_closure_unique_ptr
{
    return std::make_unique<detail::native_overload<std::remove_cvref_t<Funcs>...>>(std::make_tuple(std::forward<Funcs>(fns)...));
}

template <typename... Funcs>
auto make_shared_overload(Funcs&&... fns) -> native_closure_shared_ptr
{
    return std::make_shared<detail::native_overload<std::remove_cvref_t<Funcs>...>>(std::make_tuple(std::forward<Funcs>(fns)...));
}

}
