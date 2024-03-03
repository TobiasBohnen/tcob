// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SquirrelClosure.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

    #include "tcob/core/Common.hpp"
    #include "tcob/scripting/squirrel/SquirrelTypes.hpp"

namespace tcob::scripting::squirrel {

namespace detail {

    template <typename R, typename... Args>
    inline native_closure<R(Args...)>::native_closure(std::function<R(Args...)> fn)
        : _fn {std::move(fn)}
    {
    }

    template <typename R, typename... Args>
    inline auto native_closure<R(Args...)>::operator()(vm_view view) -> SQInteger
    {
        view.reset_error();

        std::tuple<std::remove_cvref_t<Args>...> params;
        std::apply(
            [view](auto&... item) {
                SQInteger idx {2};
                if constexpr (sizeof...(Args) > 0) {
                    using first_type = typename std::remove_cvref_t<tcob::detail::first_element_t<Args...>>;
                    if constexpr (Pointer<first_type> || std::is_same_v<stack_base, first_type>) {
                        idx = 1;
                    }
                }
                (view.pull_convert(idx, item), ...);
            },
            params);

        SQInteger const oldTop {view.get_top()};
        if constexpr (std::is_void_v<R>) {
            std::apply(_fn, params);
        } else {
            view.push_convert(std::apply(_fn, params));
        }

        if (view.has_error()) { return -1; }
        return std::max(SQInteger {0}, view.get_top() - oldTop);
    }

    ////////////////////////////////////////////////////////////

    template <typename Arg>
    inline auto compare_types_impl(vm_view view, SQInteger& startIndex) -> bool
    {
        using converter_type = std::remove_cvref_t<Arg>;
        bool const result {converter<converter_type>::IsType(view, startIndex)};
        startIndex++;
        return result;
    }

    template <typename R, typename... Args>
    inline auto compare_types(vm_view view, SQInteger startIndex, std::function<R(Args...)> const&) -> bool
    {
        return ((compare_types_impl<Args>(view, startIndex)) && ...);
    }

    ////////////////////////////////////////////////////////////

    template <typename... Funcs>
    inline native_overload<Funcs...>::native_overload(bool isStatic, std::tuple<Funcs...> fns)
        : _fns {std::move(fns)}
        , _isStatic {isStatic}
    {
    }

    template <typename... Funcs>
    inline auto native_overload<Funcs...>::operator()(vm_view view) -> SQInteger
    {
        view.reset_error();

        SQInteger const oldTop {view.get_top()};
        SQInteger const unusedStackObjects {_isStatic ? 2 : 1};

        std::apply(
            [&](auto&&... item) {
                ((check(view, oldTop - unusedStackObjects, item)) || ...);
            },
            _fns);

        if (view.has_error()) { return -1; }
        return std::max(SQInteger {0}, view.get_top() - oldTop);
    }

    template <typename... Funcs>
    template <typename R, typename T, typename... Args>
    inline auto native_overload<Funcs...>::check(vm_view view, SQInteger numArgs, R (T::*func)(Args...)) -> bool
    {
        return check_func(view, numArgs, std::function<R(T*, Args...)> {func});
    }

    template <typename... Funcs>
    inline auto native_overload<Funcs...>::check(vm_view view, SQInteger numArgs, auto&& func) -> bool
    {
        return check_func(view, numArgs, std::function {func});
    }

    template <typename... Funcs>
    template <typename R, typename... Args>
    inline auto native_overload<Funcs...>::check_func(vm_view view, SQInteger numArgs, std::function<R(Args...)> const& func) -> bool
    {
        SQInteger const startIndex {_isStatic ? 2 : 1};
        if (numArgs == sizeof...(Args) && compare_types(view, startIndex, func)) {
            call_func(view, startIndex, func);
            return true;
        }

        return false;
    }

    template <typename... Funcs>
    template <typename R, typename... Args>
    inline void native_overload<Funcs...>::call_func(vm_view view, SQInteger startIndex, std::function<R(Args...)> const& func)
    {
        std::tuple<std::remove_cvref_t<Args>...> params;
        std::apply(
            [&view, startIndex](auto&&... item) {
                [[maybe_unused]] SQInteger idx {startIndex};
                (view.pull_convert(idx, item), ...);
            },
            params);

        if constexpr (std::is_void_v<R>) {
            std::apply(func, params);
        } else {
            view.push_convert(std::apply(func, params));
        }
    }
}

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
    return std::make_unique<detail::native_overload<std::remove_cvref_t<Funcs>...>>(true, std::make_tuple(std::forward<Funcs>(fns)...));
}

template <typename... Funcs>
auto make_shared_overload(Funcs&&... fns) -> native_closure_shared_ptr
{
    return std::make_shared<detail::native_overload<std::remove_cvref_t<Funcs>...>>(true, std::make_tuple(std::forward<Funcs>(fns)...));
}

}

#endif
