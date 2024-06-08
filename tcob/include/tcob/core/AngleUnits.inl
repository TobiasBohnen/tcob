// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "AngleUnits.hpp"

#include <cmath>

namespace tcob {

template <template <typename> typename Type, typename ValueType>
constexpr angle_unit<Type, ValueType>::angle_unit(value_type value)
    : Value {value}
{
}

template <template <typename> typename Type, typename ValueType>
template <template <typename> typename Type2>
constexpr angle_unit<Type, ValueType>::angle_unit(angle_unit<Type2, ValueType> const& other) noexcept
    : Value {static_cast<ValueType>(other.Value / Type2<ValueType>::OneTurn) * Type<ValueType>::OneTurn}
{
}

template <template <typename> typename Type, typename ValueType>
template <template <typename> typename Type2, typename ValueType2>
constexpr angle_unit<Type, ValueType>::angle_unit(angle_unit<Type2, ValueType2> const& other) noexcept
    : Value {static_cast<ValueType>(other.Value / Type2<ValueType2>::OneTurn) * Type<ValueType>::OneTurn}
{
}

template <template <typename> typename Type, typename ValueType>
inline auto angle_unit<Type, ValueType>::sin() const -> value_type
{
    return std::sin(radian<value_type> {*this}.Value);
}

template <template <typename> typename Type, typename ValueType>
inline auto angle_unit<Type, ValueType>::asin() const -> value_type
{
    return std::asin(radian<value_type> {*this}.Value);
}

template <template <typename> typename Type, typename ValueType>
inline auto angle_unit<Type, ValueType>::cos() const -> value_type
{
    return std::cos(radian<value_type> {*this}.Value);
}

template <template <typename> typename Type, typename ValueType>
inline auto angle_unit<Type, ValueType>::acos() const -> value_type
{
    return std::acos(radian<value_type> {*this}.Value);
}

template <template <typename> typename Type, typename ValueType>
inline auto angle_unit<Type, ValueType>::tan() const -> value_type
{
    return std::tan(radian<value_type> {*this}.Value);
}

template <template <typename> typename Type, typename ValueType>
inline auto angle_unit<Type, ValueType>::atan() const -> value_type
{
    return std::atan(radian<value_type> {*this}.Value);
}

template <template <typename> typename Type, typename ValueType>
auto constexpr angle_unit<Type, ValueType>::as_normalized() -> Type<ValueType>
{
    return {std::fmod(this->Value, Type<value_type>::OneTurn)};
}

template <template <typename> typename Type, typename ValueType>
auto constexpr angle_unit<Type, ValueType>::Lerp(angle_unit const& left, angle_unit const& right, f64 step) -> Type<ValueType>
{
    return {static_cast<value_type>(left.Value + ((right.Value - left.Value) * step))};
}

template <template <typename> typename Type, typename ValueType>
auto constexpr operator+(angle_unit<Type, ValueType> const& left, angle_unit<Type, ValueType> const& right) -> Type<ValueType>
{
    return {left.Value + static_cast<ValueType>(right.Value)};
}

template <template <typename> typename Type, typename ValueType>
auto constexpr operator+=(angle_unit<Type, ValueType>& left, angle_unit<Type, ValueType> const& right) -> angle_unit<Type, ValueType>&
{
    left.Value += static_cast<ValueType>(right.Value);
    return left;
}

template <template <typename> typename Type, typename ValueType>
auto constexpr operator-(angle_unit<Type, ValueType> const& right) -> Type<ValueType>
{
    return {static_cast<ValueType>(-right.Value)};
}

template <template <typename> typename Type, typename ValueType>
auto constexpr operator-(angle_unit<Type, ValueType> const& left, angle_unit<Type, ValueType> const& right) -> Type<ValueType>
{
    return {left.Value - static_cast<ValueType>(right.Value)};
}

template <template <typename> typename Type, typename ValueType>
auto constexpr operator-=(angle_unit<Type, ValueType>& left, angle_unit<Type, ValueType> const& right) -> angle_unit<Type, ValueType>&
{
    left.Value -= static_cast<ValueType>(right.Value);
    return left;
}

template <template <typename> typename Type, typename ValueType, Arithmetic R>
auto constexpr operator*(angle_unit<Type, ValueType> const& left, R const& right) -> Type<ValueType>
{
    return {left.Value * static_cast<ValueType>(right)};
}

template <template <typename> typename Type, typename ValueType, Arithmetic R>
auto constexpr operator*=(angle_unit<Type, ValueType>& left, R right) -> angle_unit<Type, ValueType>&
{
    left.Value *= static_cast<ValueType>(right);
    return left;
}

template <template <typename> typename Type, typename ValueType, Arithmetic R>
auto constexpr operator/(angle_unit<Type, ValueType> const& left, R right) -> Type<ValueType>
{
    return {left.Value / static_cast<ValueType>(right)};
}

template <template <typename> typename Type, typename ValueType, Arithmetic R>
auto constexpr operator/=(angle_unit<Type, ValueType>& left, R right) -> angle_unit<Type, ValueType>&
{
    left.Value /= static_cast<ValueType>(right);
    return left;
}

template <template <typename> typename Type, typename ValueType>
auto constexpr operator==(angle_unit<Type, ValueType> const& left, angle_unit<Type, ValueType> const& right) -> bool
{
    return (left.Value == static_cast<ValueType>(right.Value));
}

template <template <typename> typename Type, typename ValueType, Arithmetic R>
auto constexpr operator==(angle_unit<Type, ValueType> const& left, R right) -> bool
{
    return (left.Value == static_cast<ValueType>(right));
}

template <template <typename> typename Type, typename ValueType>
auto constexpr operator<=>(angle_unit<Type, ValueType> const& left, angle_unit<Type, ValueType> const& right) -> std::partial_ordering
{
    return left.Value <=> static_cast<ValueType>(right.Value);
}

template <template <typename> typename Type, typename ValueType, Arithmetic R>
auto constexpr operator<=>(angle_unit<Type, ValueType> const& left, R right) -> std::partial_ordering
{
    return left.Value <=> static_cast<ValueType>(right);
}

template <template <typename> typename Type, typename ValueType>
inline auto operator<<(std::ostream& os, angle_unit<Type, ValueType> const& m) -> std::ostream&
{
    return os << "value:" << m.Value;
}

////////////////////////////////////////////////////////////

namespace literals {
    inline auto operator""_deg(unsigned long long int value) -> degree_f
    {
        return degree_f {static_cast<f32>(value)};
    }

    inline auto operator""_deg(long double value) -> degree_f
    {
        return degree_f {static_cast<f32>(value)};
    }

    inline auto operator""_rad(unsigned long long int value) -> radian_f
    {
        return radian_f {static_cast<f32>(value)};
    }

    inline auto operator""_rad(long double value) -> radian_f
    {
        return radian_f {static_cast<f32>(value)};
    }

    inline auto operator""_turn(unsigned long long int value) -> turn_f
    {
        return turn_f {static_cast<f32>(value)};
    }

    inline auto operator""_turn(long double value) -> turn_f
    {
        return turn_f {static_cast<f32>(value)};
    }

    inline auto operator""_grad(unsigned long long int value) -> gradian_f
    {
        return gradian_f {static_cast<f32>(value)};
    }

    inline auto operator""_grad(long double value) -> gradian_f
    {
        return gradian_f {static_cast<f32>(value)};
    }
}

////////////////////////////////////////////////////////////
}
