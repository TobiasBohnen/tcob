// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Wrapper.hpp"

#include <functional>
#include <utility>

#include "tcob/core/Property.hpp"

namespace tcob::scripting {

////////////////////////////////////////////////////////////

template <typename WrapperImpl>
inline wrapper<WrapperImpl>::proxy::proxy(wrapper& parent, string name)
    : _parent {parent}
    , _name {std::move(name)}
{
}

template <typename WrapperImpl>
template <typename T>
inline auto wrapper<WrapperImpl>::proxy::operator=(T const& method) -> proxy&
{
    _parent.wrap_method(_name, method);
    return *this;
}

template <typename WrapperImpl>
template <typename T>
inline auto wrapper<WrapperImpl>::proxy::operator=(getter<T> const& get) -> proxy&
{
    _parent.wrap_getter(_name, get.Method);
    return *this;
}

template <typename WrapperImpl>
template <typename T>
inline auto wrapper<WrapperImpl>::proxy::operator=(setter<T> const& set) -> proxy&
{
    _parent.wrap_setter(_name, set.Method);
    return *this;
}

template <typename WrapperImpl>
template <typename Get, typename Set>
inline auto wrapper<WrapperImpl>::proxy::operator=(property<Get, Set> const& prop) -> proxy&
{
    _parent.wrap_property(_name, prop.Getter, prop.Setter);
    return *this;
}

template <typename WrapperImpl>
template <typename... Ts>
inline auto wrapper<WrapperImpl>::proxy::operator=(overload<Ts...> const& ov) -> proxy&
{
    std::apply(
        [&](auto&&... item) { _parent.wrap_overload(_name, item...); },
        ov.Overloads);

    return *this;
}

template <typename WrapperImpl>
inline auto wrapper<WrapperImpl>::operator[](string const& name) -> proxy
{
    return proxy {*this, name};
}

////////////////////////////////////////////////////////////

template <typename WrapperImpl>
template <typename R, typename... P>
inline auto wrapper<WrapperImpl>::make_unique_closure(std::function<R(P...)>&& fn)
{
    return get_impl()->template impl_make_unique_closure<R, P...>(std::move(fn));
}

template <typename WrapperImpl>
template <typename... Funcs>
inline auto wrapper<WrapperImpl>::make_unique_overload(Funcs&&... fns)
{
    return get_impl()->template impl_make_unique_overload<Funcs...>(std::forward<Funcs>(fns)...);
}

////////////////////////////////////////////////////////////

template <typename WrapperImpl>
template <typename Func>
inline auto wrapper<WrapperImpl>::wrap_method_helper(Func&& func)
{
    return make_unique_closure(std::function {std::forward<Func>(func)});
}

template <typename WrapperImpl>
template <typename R, typename S, typename... Args>
inline auto wrapper<WrapperImpl>::wrap_method_helper(R (S::*func)(Args...))
{
    register_base<S>();
    return make_unique_closure(std::function<R(S*, Args...)> {func});
}

template <typename WrapperImpl>
template <typename R, typename S, typename... Args>
inline auto wrapper<WrapperImpl>::wrap_method_helper(R (S::*func)(Args...) const)
{
    register_base<S>();
    return make_unique_closure(std::function<R(S*, Args...)> {func});
}

template <typename WrapperImpl>
template <typename R, typename... Args>
inline auto wrapper<WrapperImpl>::wrap_method_helper(R (*func)(Args...))
{
    return make_unique_closure(std::function<R(Args...)> {func});
}

template <typename WrapperImpl>
template <typename Func>
inline auto wrapper<WrapperImpl>::wrap_property_helper(Func&& func)
{
    return make_unique_closure(std::function {std::forward<Func>(func)});
}

template <typename WrapperImpl>
template <typename R, typename S>
inline auto wrapper<WrapperImpl>::wrap_property_helper(R const (S::*prop)() const)
{
    register_base<S>();
    return make_unique_closure(std::function<R(S*)> {prop});
}

template <typename WrapperImpl>
template <typename R, typename S>
inline auto wrapper<WrapperImpl>::wrap_property_helper(R (S::*prop)() const)
{
    register_base<S>();
    return make_unique_closure(std::function<R(S*)> {prop});
}

template <typename WrapperImpl>
template <typename R, typename S>
inline auto wrapper<WrapperImpl>::wrap_property_helper(R const (S::*prop)())
{
    register_base<S>();
    return make_unique_closure(std::function<R(S*)> {prop});
}

template <typename WrapperImpl>
template <typename R, typename S>
inline auto wrapper<WrapperImpl>::wrap_property_helper(R (S::*prop)())
{
    register_base<S>();
    return make_unique_closure(std::function<R(S*)> {prop});
}

template <typename WrapperImpl>
template <typename R, typename S>
inline auto wrapper<WrapperImpl>::wrap_property_helper(void (S::*prop)(R const))
{
    register_base<S>();
    return make_unique_closure(std::function<void(S*, R)> {prop});
}

template <typename WrapperImpl>
template <typename R, typename S>
inline auto wrapper<WrapperImpl>::wrap_property_helper_field_getter(R S::* field)
{
    register_base<S>();
    auto lambda {[field](S* instance) -> R { return (instance->*field); }};
    return make_unique_closure(std::function<R(S*)> {lambda});
}

template <typename WrapperImpl>
template <typename R, typename S>
inline auto wrapper<WrapperImpl>::wrap_property_helper_field_setter(R S::* field)
{
    register_base<S>();
    auto lambda {[field](S* instance, R value) -> void { (instance->*field) = value; }};
    return make_unique_closure(std::function<void(S*, R)> {lambda});
}

template <typename WrapperImpl>
template <typename R, typename S>
inline auto wrapper<WrapperImpl>::wrap_property_helper_field_getter(prop<R> S::* prop)
{
    register_base<S>();
    auto lambda {[prop](S* instance) -> R { return (instance->*prop)(); }};
    return make_unique_closure(std::function<R(S*)> {lambda});
}

template <typename WrapperImpl>
template <typename R, typename S>
inline auto wrapper<WrapperImpl>::wrap_property_helper_field_setter(prop<R> S::* prop)
{
    register_base<S>();
    auto lambda {[prop](S* instance, R value) -> void { (instance->*prop) = value; }};
    return make_unique_closure(std::function<void(S*, R)> {lambda});
}

////////////////////////////////////////////////////////////

template <typename WrapperImpl>
inline auto wrapper<WrapperImpl>::get_impl() -> WrapperImpl*
{
    return static_cast<WrapperImpl*>(this);
}

template <typename WrapperImpl>
inline auto wrapper<WrapperImpl>::get_impl() const -> WrapperImpl const*
{
    return static_cast<WrapperImpl const*>(this);
}

////////////////////////////////////////////////////////////

template <typename WrapperImpl>
template <auto Func>
inline void wrapper<WrapperImpl>::wrap_method(string_view name)
{
    auto ptr {wrap_method_helper(Func)};
    get_impl()->impl_wrap_func(name, wrap_target::Method, std::move(ptr));
}

template <typename WrapperImpl>
inline void wrapper<WrapperImpl>::wrap_method(string_view name, auto&& func)
{
    auto ptr {wrap_method_helper(func)};
    get_impl()->impl_wrap_func(name, wrap_target::Method, std::move(ptr));
}

template <typename WrapperImpl>
template <typename... Funcs>
inline void wrapper<WrapperImpl>::wrap_overload(string_view name, Funcs&&... funcs)
{
    auto ptr {make_unique_overload<Funcs...>(std::forward<Funcs>(funcs)...)};
    get_impl()->impl_wrap_func(name, wrap_target::Method, std::move(ptr));
}

template <typename WrapperImpl>
template <auto Getter, auto Setter>
inline void wrapper<WrapperImpl>::wrap_property(string_view name)
{
    auto getter {wrap_property_helper(Getter)};
    get_impl()->impl_wrap_func(name, wrap_target::Getter, std::move(getter));
    auto setter {wrap_property_helper(Setter)};
    get_impl()->impl_wrap_func(name, wrap_target::Setter, std::move(setter));
}

template <typename WrapperImpl>
template <auto Field>
inline void wrapper<WrapperImpl>::wrap_property(string_view name)
{
    auto getter {wrap_property_helper_field_getter(Field)};
    get_impl()->impl_wrap_func(name, wrap_target::Getter, std::move(getter));
    auto setter {wrap_property_helper_field_setter(Field)};
    get_impl()->impl_wrap_func(name, wrap_target::Setter, std::move(setter));
}

template <typename WrapperImpl>
inline void wrapper<WrapperImpl>::wrap_property(string_view name, auto&& get, auto&& set)
{
    auto getter {wrap_property_helper(get)};
    get_impl()->impl_wrap_func(name, wrap_target::Getter, std::move(getter));
    auto setter {wrap_property_helper(set)};
    get_impl()->impl_wrap_func(name, wrap_target::Setter, std::move(setter));
}

template <typename WrapperImpl>
template <auto Getter>
inline void wrapper<WrapperImpl>::wrap_getter(string_view name)
{
    auto getter {wrap_property_helper(Getter)};
    get_impl()->impl_wrap_func(name, wrap_target::Getter, std::move(getter));
}

template <typename WrapperImpl>
inline void wrapper<WrapperImpl>::wrap_getter(string_view name, auto&& get)
{
    auto getter {wrap_property_helper(get)};
    get_impl()->impl_wrap_func(name, wrap_target::Getter, std::move(getter));
}

template <typename WrapperImpl>
template <auto Setter>
inline void wrapper<WrapperImpl>::wrap_setter(string_view name)
{
    auto setter {wrap_property_helper(Setter)};
    get_impl()->impl_wrap_func(name, wrap_target::Setter, std::move(setter));
}

template <typename WrapperImpl>
inline void wrapper<WrapperImpl>::wrap_setter(string_view name, auto&& set)
{
    auto setter {wrap_property_helper(set)};
    get_impl()->impl_wrap_func(name, wrap_target::Setter, std::move(setter));
}

template <typename WrapperImpl>
template <typename S>
inline void wrapper<WrapperImpl>::register_base()
{
    get_impl()->template impl_register_base<S>();
}

}
