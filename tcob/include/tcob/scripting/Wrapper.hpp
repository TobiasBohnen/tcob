// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Property.hpp"

namespace tcob::scripting {
////////////////////////////////////////////////////////////

template <typename T>
struct script_owned_ptr {
    explicit script_owned_ptr(T* obj);

    T* Pointer {nullptr};
};

////////////////////////////////////////////////////////////

template <typename T>
struct getter {
    T Method;
};

template <typename T>
struct setter {
    T Method;
};

template <typename Get, typename Set>
struct property {
    Get Getter;
    Set Setter;
};

template <typename... Ts>
struct overload {
    overload(Ts... items)
        : Overloads {items...}
    {
    }
    std::tuple<Ts...> Overloads;
};

////////////////////////////////////////////////////////////

namespace detail {
    class wrapper_base { };
}

////////////////////////////////////////////////////////////

enum class wrap_target {
    Getter,
    Setter,
    Method
};

////////////////////////////////////////////////////////////

template <typename WrapperImpl>
class wrapper : public detail::wrapper_base, public non_copyable {
public:
    class proxy {
    public:
        proxy(wrapper& parent, string name);

        template <typename T>
        auto operator=(T const& method) -> proxy&;

        template <typename T>
        auto operator=(getter<T> const& get) -> proxy&;

        template <typename T>
        auto operator=(setter<T> const& set) -> proxy&;

        template <typename Get, typename Set>
        auto operator=(property<Get, Set> const& prop) -> proxy&;

        template <typename... Ts>
        auto operator=(overload<Ts...> const& ov) -> proxy&;

    private:
        wrapper& _parent;
        string   _name;
    };

    auto operator[](string const& name) -> proxy;

    template <auto Func>
    void wrap_method(string const& name);
    void wrap_method(string const& name, auto&& func);

    template <typename... Funcs>
    void wrap_overload(string const& name, Funcs&&... funcs);

    template <auto Getter, auto Setter>
    void wrap_property(string const& name);
    template <auto Field>
    void wrap_property(string const& name);
    void wrap_property(string const& name, auto&& get, auto&& set);

    template <auto Getter>
    void wrap_getter(string const& name);
    void wrap_getter(string const& name, auto&& get);

    template <auto Setter>
    void wrap_setter(string const& name);
    void wrap_setter(string const& name, auto&& set);

    template <typename S>
    void register_base();

protected:
    template <typename R, typename... P>
    auto make_unique_closure(std::function<R(P...)>&& fn);
    template <typename... Funcs>
    auto make_unique_overload(Funcs&&... fns);

    template <typename Func>
    auto wrap_method_helper(Func&& func);
    template <typename R, typename S, typename... Args>
    auto wrap_method_helper(R (S::*func)(Args...));
    template <typename R, typename S, typename... Args>
    auto wrap_method_helper(R (S::*func)(Args...) const);
    template <typename R, typename... Args>
    auto wrap_method_helper(R (*func)(Args...));

    template <typename Func>
    auto wrap_property_helper(Func&& func);
    template <typename R, typename S>
    auto wrap_property_helper(R const (S::*prop)() const);
    template <typename R, typename S>
    auto wrap_property_helper(R (S::*prop)() const);
    template <typename R, typename S>
    auto wrap_property_helper(R const (S::*prop)());
    template <typename R, typename S>
    auto wrap_property_helper(R (S::*prop)());
    template <typename R, typename S>
    auto wrap_property_helper(void (S::*prop)(R const));
    template <typename R, typename S>
    auto wrap_property_helper_field_getter(R S::*field);
    template <typename R, typename S>
    auto wrap_property_helper_field_setter(R S::*field);
    template <typename R, typename S>
    auto wrap_property_helper_field_getter(prop<R> S::*prop);
    template <typename R, typename S>
    auto wrap_property_helper_field_setter(prop<R> S::*prop);

private:
    auto get_impl() -> WrapperImpl*;
    auto get_impl() const -> WrapperImpl const*;
};

}

#include "Wrapper.inl"
