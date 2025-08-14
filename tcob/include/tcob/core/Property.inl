// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Property.hpp"

#include <compare>

namespace tcob::detail {

template <typename T>
inline field_source<T>::field_source(type value)
    : _value {value}
{
}

template <typename T>
inline auto field_source<T>::get() -> return_type
{
    return _value;
}

template <typename T>
inline auto field_source<T>::get() const -> const_return_type
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
inline validating_field_source<T>::validating_field_source(type value, validate_func val)
    : _validate {std::move(val)}
    , _value {value}
{
}

template <typename T>
inline validating_field_source<T>::validating_field_source(validate_func val)
    : validating_field_source {{}, std::move(val)}
{
}

template <typename T>
inline auto validating_field_source<T>::get() -> return_type
{
    return _value;
}

template <typename T>
inline auto validating_field_source<T>::get() const -> const_return_type
{
    return _value;
}

template <typename T>
inline auto validating_field_source<T>::set(type const& value, bool force) -> bool
{
    T const newValue {_validate(value)};
    if constexpr (Equatable<T>) {
        if (!force && _value == newValue) { return false; }
    }
    _value = newValue;
    return true;
}

////////////////////////////////////////////////////////////

template <typename T>
inline func_source<T>::func_source(getter_func get, setter_func set)
    : _getter {std::move(get)}
    , _setter {std::move(set)}
{
}

template <typename T>
inline func_source<T>::func_source(type value, getter_func get, setter_func set)
    : func_source {std::move(get), std::move(set)}
{
    set(value);
}

template <typename T>
inline auto func_source<T>::get() -> return_type
{
    return _getter();
}

template <typename T>
inline auto func_source<T>::get() const -> const_return_type
{
    return _getter();
}

template <typename T>
inline auto func_source<T>::set(type const& value, bool force) -> bool
{
    if constexpr (Equatable<T>) {
        if (!force && get() == value) { return false; }
    }
    _setter(value);
    return true;
}

////////////////////////////////////////////////////////////

template <typename T, typename Source>
inline prop_base<T, Source>::prop_base(Source source)
    : _source {std::move(source)}
{
}

template <typename T, typename Source>
inline prop_base<T, Source>::prop_base(T val)
    : _source {val}
{
}

template <typename T, typename Source>
inline prop_base<T, Source>::operator T() const
{
    return _source.get();
}

template <typename T, typename Source>
inline auto prop_base<T, Source>::operator!() const -> bool
{
    return !_source.get();
}

template <typename T, typename Source>
inline auto prop_base<T, Source>::operator->() const
{
    if constexpr (OverloadsArrowOp<T>) {
        return _source.get();
    } else {
        static_assert(std::is_reference_v<return_type>);
        return &_source.get();
    }
}

template <typename T, typename Source>
inline auto prop_base<T, Source>::operator*() const -> const_return_type
{
    return _source.get();
}

template <typename T, typename Source>
inline void prop_base<T, Source>::operator()(T const& value)
{
    set(value, true);
}

template <typename T, typename Source>
inline auto prop_base<T, Source>::operator=(T const& value) -> prop_base&
{
    set(value, false);
    return *this;
}

template <typename T, typename Source>
inline auto prop_base<T, Source>::operator[](auto&&... idx) const -> decltype(auto)
{
    return _source.get()[idx...];
}

template <typename T, typename Source>
inline void prop_base<T, Source>::set(T const& value, bool force)
{
    if (_source.set(value, force) && Changed.slot_count() > 0) {
        Changed(_source.get());
    }
}

////////////////////////////////////////////////////////////

template <typename T, typename Source>
auto constexpr operator+=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&
{
    return left = *left + right;
}

template <typename T, typename Source>
auto constexpr operator-(prop_base<T, Source>& right) -> T
{
    return -(*right);
}

template <typename T, typename Source>
auto constexpr operator-=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&
{
    return left = *left - right;
}

template <typename T, typename Source>
auto constexpr operator/=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&
{
    return left = *left / right;
}

template <typename T, typename Source>
auto constexpr operator*=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&
{
    return left = *left * right;
}

template <typename T, typename Source>
auto constexpr operator==(prop_base<T, Source> const& left, T const& right) -> bool
{
    return *left == right;
}

template <typename T, typename Source>
auto constexpr operator==(prop_base<T, Source> const& left, prop_base<T, Source> const& right) -> bool
{
    return *left == *right;
}

template <typename T, typename Source>
auto constexpr operator<=>(prop_base<T, Source> const& left, T const& right) -> std::partial_ordering
{
    return *left <=> right;
}

template <typename T, typename Source>
auto constexpr operator<=>(prop_base<T, Source> const& left, prop_base<T, Source> const& right) -> std::partial_ordering
{
    return *left <=> *right;
}
}
