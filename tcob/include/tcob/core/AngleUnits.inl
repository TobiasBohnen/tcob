// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "AngleUnits.hpp"

#include <cmath>

namespace tcob {

template <FloatingPoint ValueType, double OneTurn>
constexpr angle_unit<ValueType, OneTurn>::angle_unit(value_type value)
    : Value {value}
{
}

template <FloatingPoint ValueType, double OneTurn>
template <FloatingPoint ValueType2, double OneTurn2>
constexpr angle_unit<ValueType, OneTurn>::angle_unit(angle_unit<ValueType2, OneTurn2> const& other) noexcept
    : Value(static_cast<ValueType>(other.Value / OneTurn2 * OneTurn))
{
}

template <FloatingPoint ValueType, double OneTurn>
inline auto angle_unit<ValueType, OneTurn>::sin() const -> value_type
{
    if constexpr (OneTurn == TAU) {
        return std::sin(Value);
    } else {
        return std::sin(angle_unit<value_type, TAU> {*this}.Value);
    }
}

template <FloatingPoint ValueType, double OneTurn>
inline auto angle_unit<ValueType, OneTurn>::asin() const -> value_type
{
    if constexpr (OneTurn == TAU) {
        return std::asin(Value);
    } else {
        return std::asin(angle_unit<value_type, TAU> {*this}.Value);
    }
}

template <FloatingPoint ValueType, double OneTurn>
inline auto angle_unit<ValueType, OneTurn>::cos() const -> value_type
{
    if constexpr (OneTurn == TAU) {
        return std::cos(Value);
    } else {
        return std::cos(angle_unit<value_type, TAU> {*this}.Value);
    }
}

template <FloatingPoint ValueType, double OneTurn>
inline auto angle_unit<ValueType, OneTurn>::acos() const -> value_type
{
    if constexpr (OneTurn == TAU) {
        return std::acos(Value);
    } else {
        return std::acos(angle_unit<value_type, TAU> {*this}.Value);
    }
}

template <FloatingPoint ValueType, double OneTurn>
inline auto angle_unit<ValueType, OneTurn>::tan() const -> value_type
{
    if constexpr (OneTurn == TAU) {
        return std::tan(Value);
    } else {
        return std::tan(angle_unit<value_type, TAU> {*this}.Value);
    }
}

template <FloatingPoint ValueType, double OneTurn>
inline auto angle_unit<ValueType, OneTurn>::atan() const -> value_type
{
    if constexpr (OneTurn == TAU) {
        return std::atan(Value);
    } else {
        return std::atan(angle_unit<value_type, TAU> {*this}.Value);
    }
}

template <FloatingPoint ValueType, double OneTurn>
auto constexpr angle_unit<ValueType, OneTurn>::as_normalized() const -> angle_unit<value_type, OneTurn>
{
    return {std::fmod(this->Value, static_cast<value_type>(OneTurn))};
}

template <FloatingPoint ValueType, double OneTurn>
auto constexpr angle_unit<ValueType, OneTurn>::Lerp(angle_unit const& left, angle_unit const& right, f64 step) -> angle_unit<value_type, OneTurn>
{
    return {static_cast<value_type>(left.Value + ((right.Value - left.Value) * step))};
}

template <FloatingPoint ValueType, double OneTurn>
auto constexpr operator+(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>
{
    return {left.Value + static_cast<ValueType>(right.Value)};
}

template <FloatingPoint ValueType, double OneTurn>
auto constexpr operator+=(angle_unit<ValueType, OneTurn>& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>&
{
    left.Value += static_cast<ValueType>(right.Value);
    return left;
}

template <FloatingPoint ValueType, double OneTurn>
auto constexpr operator-(angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>
{
    return {static_cast<ValueType>(-right.Value)};
}

template <FloatingPoint ValueType, double OneTurn>
auto constexpr operator-(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>
{
    return {left.Value - static_cast<ValueType>(right.Value)};
}

template <FloatingPoint ValueType, double OneTurn>
auto constexpr operator-=(angle_unit<ValueType, OneTurn>& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>&
{
    left.Value -= static_cast<ValueType>(right.Value);
    return left;
}

template <FloatingPoint ValueType, double OneTurn, Arithmetic R>
auto constexpr operator*(angle_unit<ValueType, OneTurn> const& left, R const& right) -> angle_unit<ValueType, OneTurn>
{
    return {left.Value * static_cast<ValueType>(right)};
}

template <FloatingPoint ValueType, double OneTurn, Arithmetic R>
auto constexpr operator*=(angle_unit<ValueType, OneTurn>& left, R right) -> angle_unit<ValueType, OneTurn>&
{
    left.Value *= static_cast<ValueType>(right);
    return left;
}

template <FloatingPoint ValueType, double OneTurn, Arithmetic R>
auto constexpr operator/(angle_unit<ValueType, OneTurn> const& left, R right) -> angle_unit<ValueType, OneTurn>
{
    return {left.Value / static_cast<ValueType>(right)};
}

template <FloatingPoint ValueType, double OneTurn, Arithmetic R>
auto constexpr operator/=(angle_unit<ValueType, OneTurn>& left, R right) -> angle_unit<ValueType, OneTurn>&
{
    left.Value /= static_cast<ValueType>(right);
    return left;
}

template <FloatingPoint ValueType, double OneTurn>
auto constexpr operator==(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> bool
{
    return (left.Value == static_cast<ValueType>(right.Value));
}

template <FloatingPoint ValueType, double OneTurn, Arithmetic R>
auto constexpr operator==(angle_unit<ValueType, OneTurn> const& left, R right) -> bool
{
    return (left.Value == static_cast<ValueType>(right));
}

template <FloatingPoint ValueType, double OneTurn>
auto constexpr operator<=>(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> std::partial_ordering
{
    return left.Value <=> static_cast<ValueType>(right.Value);
}

template <FloatingPoint ValueType, double OneTurn, Arithmetic R>
auto constexpr operator<=>(angle_unit<ValueType, OneTurn> const& left, R right) -> std::partial_ordering
{
    return left.Value <=> static_cast<ValueType>(right);
}

template <FloatingPoint ValueType, double OneTurn>
inline auto operator<<(std::ostream& os, angle_unit<ValueType, OneTurn> const& m) -> std::ostream&
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
