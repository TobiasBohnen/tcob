// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Property.hpp"

namespace tcob {

namespace detail {

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
    inline validating_field_source<T>::validating_field_source(validate_func val)
        : validating_field_source {{}, val}
    {
    }

    template <typename T>
    inline validating_field_source<T>::validating_field_source(type value, validate_func val)
        : _validate {val}
        , _value {value}
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
        : _getter {get}
        , _setter {set}
    {
    }

    template <typename T>
    inline func_source<T>::func_source(type value, getter_func get, setter_func set)
        : func_source {get, set}
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

}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

template <typename T, typename Source>
prop_base<T, Source>::prop_base() = default;

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
inline auto prop_base<T, Source>::operator=(T const& value) -> prop_base&
{
    set(value, false);
    return *this;
}

template <typename T, typename Source>
inline prop_base<T, Source>::operator T() const
{
    return _source.get();
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
inline auto prop_base<T, Source>::operator*() -> return_type
{
    return _source.get();
}

template <typename T, typename Source>
inline auto prop_base<T, Source>::operator*() const -> const_return_type
{
    return _source.get();
}

template <typename T, typename Source>
inline auto prop_base<T, Source>::operator()() const -> const_return_type
{
    return _source.get();
}

template <typename T, typename Source>
inline void prop_base<T, Source>::operator()(T const& value)
{
    set(value, true);
}

template <typename T, typename Source>
inline void prop_base<T, Source>::set(T const& value, bool force)
{
    if (_source.set(value, force) && Changed.slot_count() > 0) {
        Changed(_source.get());
    }
}

////////////////////////////////////////////////////////////

template <Container T, typename Source>
prop_base<T, Source>::prop_base() = default;

template <Container T, typename Source>
prop_base<T, Source>::prop_base(Source source)
    : _source(std::move(source))
{
}

template <Container T, typename Source>
prop_base<T, Source>::prop_base(container_type val)
    : _source(std::move(val))
{
}

template <Container T, typename Source>
prop_base<T, Source>::operator container_type() const
{
    return _source.get();
}

template <Container T, typename Source>
auto prop_base<T, Source>::operator=(container_type const& value) -> prop_base&
{
    set(value, false);
    return *this;
}

template <Container T, typename Source>
auto prop_base<T, Source>::operator->() const
{
    return &_source.get();
}

template <Container T, typename Source>
auto prop_base<T, Source>::operator*() -> return_type
{
    return _source.get();
}

template <Container T, typename Source>
auto prop_base<T, Source>::operator*() const -> const_return_type
{
    return _source.get();
}

template <Container T, typename Source>
auto prop_base<T, Source>::operator()() const -> const_return_type
{
    return _source.get();
}

template <Container T, typename Source>
void prop_base<T, Source>::add(value_type const& element)
{
    if constexpr (std::is_reference_v<return_type>) {
        auto& vec {_source.get()};
        vec.push_back(element);
        Changed(vec);
    } else {
        auto vec {_source.get()};
        vec.push_back(element);
        set(vec, true);
    }
}

template <Container T, typename Source>
void prop_base<T, Source>::set(usize index, value_type const& newValue)
{
    if constexpr (std::is_reference_v<return_type>) {
        auto& vec {_source.get()};
        if (index < vec.size()) {
            vec[index] = newValue;
            Changed(vec);
        }
    } else {
        auto vec {_source.get()};
        if (index < vec.size()) {
            vec[index] = newValue;
            set(vec, true);
        }
    }
}

template <Container T, typename Source>
void prop_base<T, Source>::set(container_type const& value, bool force)
{
    if (_source.set(value, force)) {
        Changed(value);
    }
}

////////////////////////////////////////////////////////////

template <typename T, typename Source>
auto constexpr operator+=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&
{
    return left = left() + right;
}

template <typename T, typename Source>
auto constexpr operator+=(prop_base<T const, Source>& left, T const& right) -> prop_base<T const, Source>&
{
    return left = left() + right;
}

template <typename T, typename Source>
auto constexpr operator-(prop_base<T, Source>& right) -> T
{
    return -right();
}

template <typename T, typename Source>
auto constexpr operator-=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&
{
    return left = left() - right;
}

template <typename T, typename Source>
auto constexpr operator-=(prop_base<T const, Source>& left, T const& right) -> prop_base<T const, Source>&
{
    return left = left() - right;
}

template <typename T, typename Source>
auto constexpr operator/=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&
{
    return left = left() / right;
}

template <typename T, typename Source>
auto constexpr operator/=(prop_base<T const, Source>& left, T const& right) -> prop_base<T const, Source>&
{
    return left = left() / right;
}

template <typename T, typename Source>
auto constexpr operator*=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&
{
    return left = left() * right;
}

template <typename T, typename Source>
auto constexpr operator*=(prop_base<T const, Source>& left, T const& right) -> prop_base<T const, Source>&
{
    return left = left() * right;
}

template <typename T, typename Source>
auto constexpr operator==(prop_base<T, Source> const& left, T const& right) -> bool
{
    return left() == right;
}

template <typename T, typename Source>
auto constexpr operator==(prop_base<T const, Source> const& left, T const& right) -> bool
{
    return left() == right;
}

template <typename T, typename Source>
auto constexpr operator==(prop_base<T, Source> const& left, prop_base<T, Source> const& right) -> bool
{
    return left() == right();
}

template <typename T, typename Source>
auto constexpr operator<=>(prop_base<T, Source> const& left, T const& right) -> std::partial_ordering
{
    return left() <=> right;
}

template <typename T, typename Source>
auto constexpr operator<=>(prop_base<T const, Source> const& left, T const& right) -> std::partial_ordering
{
    return left() <=> right;
}

template <typename T, typename Source>
auto constexpr operator<=>(prop_base<T, Source> const& left, prop_base<T, Source> const& right) -> std::partial_ordering
{
    return left() <=> right();
}
}
