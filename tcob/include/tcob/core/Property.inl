// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Property.hpp"

#include <compare>
#include <type_traits>
#include <utility>

namespace tcob::detail {

template <typename T>
inline field_source<T>::field_source(type value)
    : _value {value}
{
}

template <typename T>
inline auto field_source<T>::get() noexcept -> return_type
{
    return _value;
}

template <typename T>
inline auto field_source<T>::get() const noexcept -> const_return_type
{
    return _value;
}

template <typename T>
inline auto field_source<T>::set(type const& value, bool force) -> bool
{
    if constexpr (Equatable<T>) {
        if (!force && _value == value) { return false; }
    }
    _value = value;
    return true;
}

////////////////////////////////////////////////////////////

template <typename T>
inline checked_field_source<T>::checked_field_source(chk_func func)
    : _check {std::move(func)}
{
}

template <typename T>
inline auto checked_field_source<T>::get() noexcept -> return_type
{
    return _value;
}

template <typename T>
inline auto checked_field_source<T>::get() const noexcept -> const_return_type
{
    return _value;
}

template <typename T>
inline auto checked_field_source<T>::set(type const& value, bool force) -> bool
{
    T const newValue {_check(value)};
    if constexpr (Equatable<T>) {
        if (!force && _value == newValue) { return false; }
    }
    _value = newValue;
    return true;
}

////////////////////////////////////////////////////////////

template <typename T>
inline func_source<T>::func_source(void* ctx, getter_func g, setter_func s)
    : _ctx {ctx}
    , _getter {g}
    , _setter {s}
{
}
template <typename T>
inline auto func_source<T>::get() noexcept -> return_type
{
    return _getter(_ctx);
}
template <typename T>
inline auto func_source<T>::get() const noexcept -> const_return_type
{
    return _getter(_ctx);
}

template <typename T>
inline auto func_source<T>::set(type const& value, bool force) -> bool
{
    if constexpr (Equatable<T>) {
        if (!force && get() == value) { return false; }
    }
    _setter(_ctx, value);
    return true;
}

////////////////////////////////////////////////////////////

template <typename T, typename Source>
constexpr prop<T, Source>::prop(T val)
    : _source {val}
{
}

template <typename T, typename Source>
constexpr prop<T, Source>::prop(Source source)
    : _source {std::move(source)}
{
}

template <typename T, typename Source>
inline prop<T, Source>::operator T() const
{
    return _source.get();
}

template <typename T, typename Source>
inline auto prop<T, Source>::operator!() const -> bool
{
    return !_source.get();
}

template <typename T, typename Source>
inline auto prop<T, Source>::operator->() const
{
    if constexpr (OverloadsArrowOp<T> || std::is_pointer_v<T>) {
        return _source.get();
    } else {
        static_assert(std::is_reference_v<return_type>);
        if constexpr (std::is_reference_v<return_type>) {
            return &_source.get();
        }
        std::unreachable();
    }
}

template <typename T, typename Source>
inline auto prop<T, Source>::operator*() const noexcept -> const_return_type
{
    return _source.get();
}

template <typename T, typename Source>
inline void prop<T, Source>::operator()(T const& value)
{
    set(value, true);
}

template <typename T, typename Source>
inline auto prop<T, Source>::operator=(T const& value) -> prop&
{
    set(value, false);
    return *this;
}

template <typename T, typename Source>
inline auto prop<T, Source>::operator[](auto&&... idx) const noexcept -> decltype(auto)
{
    return _source.get()[idx...];
}

template <typename T, typename Source>
inline void prop<T, Source>::mutate(auto&& func)
{
    T copy {_source.get()};
    using result_t = std::invoke_result_t<decltype(func), T&>;

    if constexpr (std::is_same_v<result_t, bool>) {
        if (func(copy)) {
            set(copy, false);
        }
    } else if constexpr (std::is_void_v<result_t>) {
        func(copy);
        set(copy, false);
    }
}

template <typename T, typename Source>
inline void prop<T, Source>::bind(auto&... others)
{
    Changed.connect([&others...](T const& v) { ((others = v), ...); });
    ((others.Changed.connect([this](T const& v) { *this = v; })), ...);
}

template <typename T, typename Source>
inline void prop<T, Source>::set(T const& value, bool force)
{
    if (_source.set(value, force)
        && Changed.slot_count() > 0) { // avoid unnecessary call to _source.get()
        Changed(_source.get());
    }
}

////////////////////////////////////////////////////////////

template <typename T, typename Source>
auto constexpr operator+=(prop<T, Source>& left, T const& right) -> prop<T, Source>&
{
    return left = *left + right;
}

template <typename T, typename Source>
auto constexpr operator-(prop<T, Source>& right) -> T
{
    return -(*right);
}

template <typename T, typename Source>
auto constexpr operator-=(prop<T, Source>& left, T const& right) -> prop<T, Source>&
{
    return left = *left - right;
}

template <typename T, typename Source>
auto constexpr operator/=(prop<T, Source>& left, T const& right) -> prop<T, Source>&
{
    return left = *left / right;
}

template <typename T, typename Source>
auto constexpr operator*=(prop<T, Source>& left, T const& right) -> prop<T, Source>&
{
    return left = *left * right;
}

template <typename T, typename Source>
auto constexpr operator==(prop<T, Source> const& left, T const& right) -> bool
{
    return *left == right;
}

template <typename T, typename Source>
auto constexpr operator==(prop<T, Source> const& left, prop<T, Source> const& right) -> bool
{
    return *left == *right;
}

template <typename T, typename Source>
auto constexpr operator<=>(prop<T, Source> const& left, T const& right) -> std::partial_ordering
{
    return *left <=> right;
}

template <typename T, typename Source>
auto constexpr operator<=>(prop<T, Source> const& left, prop<T, Source> const& right) -> std::partial_ordering
{
    return *left <=> *right;
}
}
