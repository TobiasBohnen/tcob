// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "AngleUnits.hpp"

#include <cmath>
#include <compare>
#include <ostream>

namespace tcob {

template <FloatingPoint ValueType, f64 OneTurn>
constexpr angle_unit<ValueType, OneTurn>::angle_unit(ValueType value)
    : Value {value}
{
}

template <FloatingPoint ValueType, f64 OneTurn>
template <FloatingPoint ValueType2, f64 OneTurn2>
constexpr angle_unit<ValueType, OneTurn>::angle_unit(angle_unit<ValueType2, OneTurn2> const& other) noexcept
    : Value(static_cast<ValueType>(static_cast<f64>(other.Value) / OneTurn2 * OneTurn))
{
}

template <FloatingPoint ValueType, f64 OneTurn>
inline auto angle_unit<ValueType, OneTurn>::sin() const -> ValueType
{
    if constexpr (OneTurn == TAU) {
        return std::sin(Value);
    } else {
        return std::sin(angle_unit<ValueType, TAU> {*this}.Value);
    }
}

template <FloatingPoint ValueType, f64 OneTurn>
inline auto angle_unit<ValueType, OneTurn>::asin() const -> ValueType
{
    if constexpr (OneTurn == TAU) {
        return std::asin(Value);
    } else {
        return std::asin(angle_unit<ValueType, TAU> {*this}.Value);
    }
}

template <FloatingPoint ValueType, f64 OneTurn>
inline auto angle_unit<ValueType, OneTurn>::cos() const -> ValueType
{
    if constexpr (OneTurn == TAU) {
        return std::cos(Value);
    } else {
        return std::cos(angle_unit<ValueType, TAU> {*this}.Value);
    }
}

template <FloatingPoint ValueType, f64 OneTurn>
inline auto angle_unit<ValueType, OneTurn>::acos() const -> ValueType
{
    if constexpr (OneTurn == TAU) {
        return std::acos(Value);
    } else {
        return std::acos(angle_unit<ValueType, TAU> {*this}.Value);
    }
}

template <FloatingPoint ValueType, f64 OneTurn>
inline auto angle_unit<ValueType, OneTurn>::tan() const -> ValueType
{
    if constexpr (OneTurn == TAU) {
        return std::tan(Value);
    } else {
        return std::tan(angle_unit<ValueType, TAU> {*this}.Value);
    }
}

template <FloatingPoint ValueType, f64 OneTurn>
inline auto angle_unit<ValueType, OneTurn>::atan() const -> ValueType
{
    if constexpr (OneTurn == TAU) {
        return std::atan(Value);
    } else {
        return std::atan(angle_unit<ValueType, TAU> {*this}.Value);
    }
}

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr angle_unit<ValueType, OneTurn>::as_normalized(angle_normalize mode) const -> angle_unit<ValueType, OneTurn>
{
    auto result {std::fmod(this->Value, one_turn)};

    switch (mode) {
    case angle_normalize::FullTurnSymmetric: break;
    case angle_normalize::HalfTurnSymmetric:
        result = std::fmod(result + one_turn, one_turn);
        if (result > one_turn / 2) { result -= one_turn; }
        break;
    case angle_normalize::PositiveFullTurn:
        if (result < 0) { result += one_turn; }
        break;
    }

    return angle_unit<ValueType, OneTurn> {result};
}

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr angle_unit<ValueType, OneTurn>::equals(angle_unit const& other, value_type tol) const -> bool
{
    if (*this == other) { return true; }
    auto const diff {std::abs((*this - other).as_normalized().Value)};
    return diff <= tol || diff >= (OneTurn - tol);
}

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr angle_unit<ValueType, OneTurn>::Lerp(angle_unit const& left, angle_unit const& right, f64 step) -> angle_unit<ValueType, OneTurn>
{
    ValueType const leftVal {static_cast<ValueType>(left.Value)};
    ValueType const rightVal {static_cast<ValueType>(right.Value)};
    return angle_unit<ValueType, OneTurn> {leftVal + (rightVal - leftVal) * static_cast<ValueType>(step)};
}

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr operator+(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>
{
    return angle_unit<ValueType, OneTurn> {left.Value + static_cast<ValueType>(right.Value)};
}

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr operator+=(angle_unit<ValueType, OneTurn>& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>&
{
    left.Value += static_cast<ValueType>(right.Value);
    return left;
}

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr operator-(angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>
{
    return angle_unit<ValueType, OneTurn> {static_cast<ValueType>(-right.Value)};
}

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr operator-(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>
{
    return angle_unit<ValueType, OneTurn> {left.Value - static_cast<ValueType>(right.Value)};
}

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr operator-=(angle_unit<ValueType, OneTurn>& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>&
{
    left.Value -= static_cast<ValueType>(right.Value);
    return left;
}

template <FloatingPoint ValueType, f64 OneTurn, Arithmetic R>
auto constexpr operator*(angle_unit<ValueType, OneTurn> const& left, R const& right) -> angle_unit<ValueType, OneTurn>
{
    return angle_unit<ValueType, OneTurn> {left.Value * static_cast<ValueType>(right)};
}

template <FloatingPoint ValueType, f64 OneTurn, Arithmetic R>
auto constexpr operator*=(angle_unit<ValueType, OneTurn>& left, R right) -> angle_unit<ValueType, OneTurn>&
{
    left.Value *= static_cast<ValueType>(right);
    return left;
}

template <FloatingPoint ValueType, f64 OneTurn, Arithmetic R>
auto constexpr operator/(angle_unit<ValueType, OneTurn> const& left, R right) -> angle_unit<ValueType, OneTurn>
{
    return angle_unit<ValueType, OneTurn> {left.Value / static_cast<ValueType>(right)};
}

template <FloatingPoint ValueType, f64 OneTurn, Arithmetic R>
auto constexpr operator/=(angle_unit<ValueType, OneTurn>& left, R right) -> angle_unit<ValueType, OneTurn>&
{
    left.Value /= static_cast<ValueType>(right);
    return left;
}

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr operator==(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> bool
{
    return left.Value == static_cast<ValueType>(right.Value);
}

template <FloatingPoint ValueType, f64 OneTurn, Arithmetic R>
auto constexpr operator==(angle_unit<ValueType, OneTurn> const& left, R right) -> bool
{
    return left.Value == static_cast<ValueType>(right);
}

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr operator<=>(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> std::partial_ordering
{
    return left.Value <=> static_cast<ValueType>(right.Value);
}

template <FloatingPoint ValueType, f64 OneTurn, Arithmetic R>
auto constexpr operator<=>(angle_unit<ValueType, OneTurn> const& left, R right) -> std::partial_ordering
{
    return left.Value <=> static_cast<ValueType>(right);
}

template <FloatingPoint ValueType, f64 OneTurn>
inline auto operator<<(std::ostream& os, angle_unit<ValueType, OneTurn> const& m) -> std::ostream&
{
    return os << "value:" << m.Value;
}

////////////////////////////////////////////////////////////

namespace literals {
    inline auto operator""_deg(unsigned long long int value) -> degree_f
    {
        return degree_f {static_cast<float>(value)};
    }

    inline auto operator""_deg(long double value) -> degree_f
    {
        return degree_f {static_cast<float>(value)};
    }

    inline auto operator""_rad(unsigned long long int value) -> radian_f
    {
        return radian_f {static_cast<float>(value)};
    }

    inline auto operator""_rad(long double value) -> radian_f
    {
        return radian_f {static_cast<float>(value)};
    }

    inline auto operator""_turn(unsigned long long int value) -> turn_f
    {
        return turn_f {static_cast<float>(value)};
    }

    inline auto operator""_turn(long double value) -> turn_f
    {
        return turn_f {static_cast<float>(value)};
    }

    inline auto operator""_grad(unsigned long long int value) -> gradian_f
    {
        return gradian_f {static_cast<float>(value)};
    }

    inline auto operator""_grad(long double value) -> gradian_f
    {
        return gradian_f {static_cast<float>(value)};
    }
}

////////////////////////////////////////////////////////////
}
