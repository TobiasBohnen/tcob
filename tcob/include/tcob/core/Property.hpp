// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <compare>
#include <functional>
#include <type_traits>

#include "tcob/core/Concepts.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Signal.hpp"

namespace tcob {

namespace detail {
    ////////////////////////////////////////////////////////////

    template <typename T>
    class field_source final {
    public:
        using type              = std::remove_const_t<T>;
        using return_type       = type&;
        using const_return_type = type const&;

        field_source() = default;
        field_source(type value);

        auto get() noexcept -> return_type;
        auto get() const noexcept -> const_return_type;
        auto set(type const& value, bool force) -> bool;

    private:
        type _value {};
    };

    ////////////////////////////////////////////////////////////

    template <typename T>
    class validating_field_source final {
    public:
        using type              = std::remove_const_t<T>;
        using return_type       = type&;
        using const_return_type = type const&;

        using validate_func = std::function<type(type const&)>;

        validating_field_source(validate_func val);

        auto get() noexcept -> return_type;
        auto get() const noexcept -> const_return_type;
        auto set(type const& value, bool force) -> bool;

    private:
        validate_func _validate;
        type          _value;
    };

    ////////////////////////////////////////////////////////////

    template <typename T>
    class func_source final {
    public:
        using type              = T;
        using return_type       = T;
        using const_return_type = T const;

        using getter_func = T (*)(void*);
        using setter_func = void (*)(void*, const_return_type&);

        func_source(void* ctx, getter_func g, setter_func s);

        auto get() noexcept -> return_type;
        auto get() const noexcept -> const_return_type;
        auto set(type const& value, bool force) -> bool;

    private:
        void*       _ctx;
        getter_func _getter;
        setter_func _setter;
    };

}

////////////////////////////////////////////////////////////

namespace detail {
    template <typename T, typename Source>
    class prop_base : public non_copyable {
    public:
        using return_type       = typename Source::return_type;
        using const_return_type = typename Source::const_return_type;

        prop_base() = default;
        explicit prop_base(T val); // HACK: only field_source
        explicit prop_base(Source source);

        signal<T const> Changed;

             operator T() const;
        auto operator!() const -> bool;
        auto operator->() const;
        auto operator*() const noexcept -> const_return_type;

        void operator()(T const& value);
        auto operator=(T const& value) -> prop_base&;

        auto operator[](auto&&... idx) const noexcept -> decltype(auto);

        void mutate(auto&& func) noexcept;

        void bind(auto&... others);

    protected:
        void set(T const& value, bool force);

        Source _source;
    };

    ////////////////////////////////////////////////////////////

    template <typename T, typename Source>
    auto constexpr operator+=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&;

    template <typename T, typename Source>
    auto constexpr operator-(prop_base<T, Source>& right) -> T;

    template <typename T, typename Source>
    auto constexpr operator-=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&;

    template <typename T, typename Source>
    auto constexpr operator/=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&;

    template <typename T, typename Source>
    auto constexpr operator*=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&;

    template <typename T, typename Source>
    auto constexpr operator==(prop_base<T, Source> const& left, T const& right) -> bool;

    template <typename T, typename Source>
    auto constexpr operator==(prop_base<T, Source> const& left, prop_base<T, Source> const& right) -> bool;

    template <typename T, typename Source>
    auto constexpr operator<=>(prop_base<T, Source> const& left, T const& right) -> std::partial_ordering;

    template <typename T, typename Source>
    auto constexpr operator<=>(prop_base<T, Source> const& left, prop_base<T, Source> const& right) -> std::partial_ordering;

    template <typename T>
    struct is_prop_base : std::false_type { };

    template <typename V, typename S>
    struct is_prop_base<prop_base<V, S>> : std::true_type { };

}

////////////////////////////////////////////////////////////

// field-backed property.
template <typename T>
using prop = detail::prop_base<T, detail::field_source<T>>;

// validating field-backed property.
template <typename T>
using prop_val = detail::prop_base<T, detail::validating_field_source<T>>;

// function-backed property.
template <typename T>
using prop_fn = detail::prop_base<T, detail::func_source<T>>;

template <typename T>
concept PropertyLike = detail::is_prop_base<std::remove_cvref_t<T>>::value;

template <typename T, auto Getter, auto Setter, typename Parent>
auto make_prop_fn(Parent* owner) -> prop_fn<T>
{
    if constexpr (std::is_member_function_pointer_v<decltype(Getter)> && std::is_member_function_pointer_v<decltype(Setter)>) {
        return prop_fn<T> {{owner,
                            [](void* ctx) { return (static_cast<Parent*>(ctx)->*Getter)(); },
                            [](void* ctx, T const& value) { (static_cast<Parent*>(ctx)->*Setter)(value); }}};
    } else {
        return prop_fn<T> {{owner,
                            [](void* ctx) { return Getter(static_cast<Parent*>(ctx)); },
                            [](void* ctx, T const& value) { Setter(static_cast<Parent*>(ctx), value); }}};
    }
}

}

#include "Property.inl"
