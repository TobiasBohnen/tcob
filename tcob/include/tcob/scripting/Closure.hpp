// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <memory>
#include <tuple>

#include "tcob/core/Interfaces.hpp"
#include "tcob/scripting/Lua.hpp"

namespace tcob::scripting {

namespace detail {
    ////////////////////////////////////////////////////////////

    class TCOB_API native_closure_base : public non_copyable {
    public:
        native_closure_base()          = default;
        virtual ~native_closure_base() = default;

        virtual auto operator()(state_view view) -> i32 = 0;
    };

    ////////////////////////////////////////////////////////////

    template <typename R, typename... Args>
    class native_closure;
    template <typename R, typename... Args>
    class native_closure<R(Args...)> : public native_closure_base {
    public:
        explicit native_closure(std::function<R(Args...)> fn);

        auto operator()(state_view view) -> i32 override;

    private:
        std::function<R(Args...)> _fn;
    };

    ////////////////////////////////////////////////////////////

    template <typename... Funcs>
    class native_overload : public native_closure_base {
    public:
        explicit native_overload(std::tuple<Funcs...> fns);

        auto operator()(state_view view) -> i32 override;

    private:
        template <typename R, typename T, typename... Args>
        auto try_call(state_view view, i32 top, R (T::*func)(Args...)) -> bool;
        auto try_call(state_view view, i32 top, auto&& func) -> bool;
        template <typename R, typename... Args>
        auto try_call_func(state_view view, i32 top, std::function<R(Args...)> const& func) -> bool;

        std::tuple<Funcs...> _fns;
    };

}

////////////////////////////////////////////////////////////

using native_closure_unique_ptr = std::unique_ptr<detail::native_closure_base>;
using native_closure_shared_ptr = std::shared_ptr<detail::native_closure_base>;

template <typename R, typename... P>
auto make_unique_closure(std::function<R(P...)>&& fn) -> native_closure_unique_ptr;
template <typename R, typename... P>
auto make_shared_closure(std::function<R(P...)>&& fn) -> native_closure_shared_ptr;

template <typename... Funcs>
auto make_unique_overload(Funcs&&... fns) -> native_closure_unique_ptr;
template <typename... Funcs>
auto make_shared_overload(Funcs&&... fns) -> native_closure_shared_ptr;
}

#include "Closure.inl"
