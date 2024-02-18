// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

    #include "tcob/core/Interfaces.hpp"
    #include "tcob/scripting/squirrel/Squirrel.hpp"

namespace tcob::scripting::squirrel {

namespace detail {
    ////////////////////////////////////////////////////////////

    class TCOB_API native_closure_base : public non_copyable {
    public:
        native_closure_base()          = default;
        virtual ~native_closure_base() = default;

        auto virtual operator()(vm_view view) -> SQInteger = 0;
    };

    ////////////////////////////////////////////////////////////

    template <typename R, typename... Args>
    class native_closure;
    template <typename R, typename... Args>
    class native_closure<R(Args...)> : public native_closure_base {
    public:
        explicit native_closure(std::function<R(Args...)> fn);

        auto operator()(vm_view view) -> SQInteger override;

    private:
        std::function<R(Args...)> _fn;
    };

    ////////////////////////////////////////////////////////////

    template <typename... Funcs>
    class native_overload : public native_closure_base {
    public:
        explicit native_overload(bool isStatic, std::tuple<Funcs...> fns);

        auto operator()(vm_view view) -> SQInteger override;

    private:
        template <typename R, typename T, typename... Args>
        auto check(vm_view view, SQInteger numArgs, R (T::*func)(Args...)) -> bool;
        auto check(vm_view view, SQInteger numArgs, auto&& func) -> bool;
        template <typename R, typename... Args>
        auto check_func(vm_view view, SQInteger numArgs, std::function<R(Args...)> const& func) -> bool;

        template <typename R, typename... Args>
        auto call_func(vm_view view, SQInteger startIndex, std::function<R(Args...)> const& func) -> void;

        std::tuple<Funcs...> _fns;
        bool                 _isStatic;
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

    #include "SquirrelClosure.inl"

#endif
