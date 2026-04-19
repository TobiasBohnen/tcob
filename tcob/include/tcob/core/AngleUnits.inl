// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "AngleUnits.hpp"

#include <cmath>
#include <compare>

#include "tcob/core/Common.hpp"

namespace tcob {

template <FloatingPoint ValueType, f64 OneTurn>
constexpr angle_unit<ValueType, OneTurn>::angle_unit(ValueType value)
    : Value {value}
{
}

template <FloatingPoint ValueType, f64 OneTurn>
template <FloatingPoint ValueType2>
constexpr angle_unit<ValueType, OneTurn>::angle_unit(angle_unit<ValueType2, OneTurn> const& other) noexcept
    : Value {static_cast<ValueType>(static_cast<f64>(other.Value))}
{
}

template <FloatingPoint ValueType, f64 OneTurn>
template <FloatingPoint ValueType2, f64 OneTurn2>
constexpr angle_unit<ValueType, OneTurn>::angle_unit(angle_unit<ValueType2, OneTurn2> const& other) noexcept
    : Value {static_cast<ValueType>(static_cast<f64>(other.Value) / OneTurn2 * OneTurn)}
{
}

template <FloatingPoint ValueType, f64 OneTurn>
inline auto angle_unit<ValueType, OneTurn>::sin() const -> ValueType
{
    if constexpr (OneTurn == TAU) {
        return std::sin(Value);
    } else {
        return static_cast<ValueType>(std::sin(Value / OneTurn * TAU));
    }
}

template <FloatingPoint ValueType, f64 OneTurn>
inline auto angle_unit<ValueType, OneTurn>::cos() const -> ValueType
{
    if constexpr (OneTurn == TAU) {
        return std::cos(Value);
    } else {
        return static_cast<ValueType>(std::cos(Value / OneTurn * TAU));
    }
}

template <FloatingPoint ValueType, f64 OneTurn>
inline auto angle_unit<ValueType, OneTurn>::tan() const -> ValueType
{
    if constexpr (OneTurn == TAU) {
        return std::tan(Value);
    } else {
        return static_cast<ValueType>(std::tan(Value / OneTurn * TAU));
    }
}

template <FloatingPoint ValueType, f64 OneTurn>
inline auto angle_unit<ValueType, OneTurn>::asin(ValueType ratio) -> angle_unit<ValueType, OneTurn>
{
    ValueType const radians {static_cast<ValueType>(std::asin(ratio))};
    if constexpr (OneTurn == TAU) {
        return angle_unit<ValueType, OneTurn> {radians};
    } else {
        return angle_unit<ValueType, OneTurn> {static_cast<ValueType>(radians * OneTurn / TAU)};
    }
}

template <FloatingPoint ValueType, f64 OneTurn>
inline auto angle_unit<ValueType, OneTurn>::acos(ValueType ratio) -> angle_unit<ValueType, OneTurn>
{
    ValueType const radians {static_cast<ValueType>(std::acos(ratio))};
    if constexpr (OneTurn == TAU) {
        return angle_unit<ValueType, OneTurn> {radians};
    } else {
        return angle_unit<ValueType, OneTurn> {static_cast<ValueType>(radians * OneTurn / TAU)};
    }
}

template <FloatingPoint ValueType, f64 OneTurn>
inline auto angle_unit<ValueType, OneTurn>::atan(ValueType ratio) -> angle_unit<ValueType, OneTurn>
{
    ValueType const radians {static_cast<ValueType>(std::atan(ratio))};
    if constexpr (OneTurn == TAU) {
        return angle_unit<ValueType, OneTurn> {radians};
    } else {
        return angle_unit<ValueType, OneTurn> {static_cast<ValueType>(radians * OneTurn / TAU)};
    }
}

template <FloatingPoint ValueType, f64 OneTurn>
inline auto angle_unit<ValueType, OneTurn>::atan2(ValueType y, ValueType x) -> angle_unit<ValueType, OneTurn>
{
    ValueType const radians {static_cast<ValueType>(std::atan2(y, x))};
    if constexpr (OneTurn == TAU) {
        return angle_unit<ValueType, OneTurn> {radians};
    } else {
        return angle_unit<ValueType, OneTurn> {static_cast<ValueType>(radians * OneTurn / TAU)};
    }
}

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr angle_unit<ValueType, OneTurn>::as_normalized(angle_normalize mode) const -> angle_unit<ValueType, OneTurn>
{
    ValueType retValue {};

    switch (mode) {
    case angle_normalize::FullTurnSymmetric: {
        retValue = std::fmod(this->Value, one_turn);
        break;
    }

    case angle_normalize::HalfTurnSymmetric: {
        retValue = std::fmod(this->Value + one_turn, one_turn);
        if (retValue >= one_turn / 2) { retValue -= one_turn; }
        break;
    }

    case angle_normalize::PositiveFullTurn: {
        retValue = std::fmod(this->Value, one_turn);
        if (retValue < 0) { retValue += one_turn; }
        break;
    }
    }

    return angle_unit<ValueType, OneTurn> {retValue};
}

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr angle_unit<ValueType, OneTurn>::equals(angle_unit const& other, value_type tol) const -> bool
{
    if (*this == other) { return true; }
    auto const diff {(*this - other).as_normalized(angle_normalize::PositiveFullTurn).Value};
    return diff <= tol || diff >= (OneTurn - tol);
}

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr angle_unit<ValueType, OneTurn>::Lerp(angle_unit const& from, angle_unit const& to, f64 step) -> angle_unit<ValueType, OneTurn>
{
    ValueType const leftVal {static_cast<ValueType>(from.Value)};
    ValueType const rightVal {static_cast<ValueType>(to.Value)};
    return angle_unit<ValueType, OneTurn> {helper::lerp(leftVal, rightVal, step)};
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
