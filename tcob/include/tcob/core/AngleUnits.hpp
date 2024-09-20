// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Concepts.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <FloatingPoint ValueType, double OneTurn>
class [[nodiscard]] angle_unit {
public:
    using value_type = ValueType;

    constexpr angle_unit() = default;
    constexpr angle_unit(value_type value);

    template <FloatingPoint ValueType2, double OneTurn2>
    constexpr angle_unit(angle_unit<ValueType2, OneTurn2> const& other) noexcept;

    auto sin [[nodiscard]] () const -> value_type;
    auto asin [[nodiscard]] () const -> value_type;
    auto cos [[nodiscard]] () const -> value_type;
    auto acos [[nodiscard]] () const -> value_type;
    auto tan [[nodiscard]] () const -> value_type;
    auto atan [[nodiscard]] () const -> value_type;

    auto constexpr as_normalized [[nodiscard]] () const -> angle_unit<ValueType, OneTurn>;

    auto static constexpr Lerp(angle_unit const& left, angle_unit const& right, f64 step) -> angle_unit<ValueType, OneTurn>;

    value_type Value {0};
};

template <FloatingPoint ValueType, double OneTurn>
auto constexpr operator+(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>;

template <FloatingPoint ValueType, double OneTurn>
auto constexpr operator+=(angle_unit<ValueType, OneTurn>& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>&;

template <FloatingPoint ValueType, double OneTurn>
auto constexpr operator-(angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>;

template <FloatingPoint ValueType, double OneTurn>
auto constexpr operator-(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>;

template <FloatingPoint ValueType, double OneTurn>
auto constexpr operator-=(angle_unit<ValueType, OneTurn>& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>&;

template <FloatingPoint ValueType, double OneTurn, Arithmetic R>
auto constexpr operator*(angle_unit<ValueType, OneTurn> const& left, R const& right) -> angle_unit<ValueType, OneTurn>;

template <FloatingPoint ValueType, double OneTurn, Arithmetic R>
auto constexpr operator*=(angle_unit<ValueType, OneTurn>& left, R right) -> angle_unit<ValueType, OneTurn>&;

template <FloatingPoint ValueType, double OneTurn, Arithmetic R>
auto constexpr operator/(angle_unit<ValueType, OneTurn> const& left, R right) -> angle_unit<ValueType, OneTurn>;

template <FloatingPoint ValueType, double OneTurn, Arithmetic R>
auto constexpr operator/=(angle_unit<ValueType, OneTurn>& left, R right) -> angle_unit<ValueType, OneTurn>&;

template <FloatingPoint ValueType, double OneTurn>
auto constexpr operator==(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> bool;

template <FloatingPoint ValueType, double OneTurn, Arithmetic R>
auto constexpr operator==(angle_unit<ValueType, OneTurn> const& left, R right) -> bool;

template <FloatingPoint ValueType, double OneTurn>
auto constexpr operator<=>(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> std::partial_ordering;

template <FloatingPoint ValueType, double OneTurn, Arithmetic R>
auto constexpr operator<=>(angle_unit<ValueType, OneTurn> const& left, R right) -> std::partial_ordering;

template <FloatingPoint ValueType, double OneTurn>
inline auto operator<<(std::ostream& os, angle_unit<ValueType, OneTurn> const& m) -> std::ostream&;

////////////////////////////////////////////////////////////

using degree_f = angle_unit<f32, 360.>;
using degree_d = angle_unit<f64, 360.>;

////////////////////////////////////////////////////////////

using radian_f = angle_unit<f32, TAU>;
using radian_d = angle_unit<f64, TAU>;

////////////////////////////////////////////////////////////

using turn_f = angle_unit<f32, 1.>;
using turn_d = angle_unit<f64, 1.>;

////////////////////////////////////////////////////////////

using gradian_f = angle_unit<f32, 400.>;
using gradian_d = angle_unit<f64, 400.>;

////////////////////////////////////////////////////////////

namespace literals {

    inline auto operator""_deg(unsigned long long int value) -> degree_f;
    inline auto operator""_deg(long double value) -> degree_f;

    inline auto operator""_rad(unsigned long long int value) -> radian_f;
    inline auto operator""_rad(long double value) -> radian_f;

    inline auto operator""_turn(unsigned long long int value) -> turn_f;
    inline auto operator""_turn(long double value) -> turn_f;

    inline auto operator""_grad(unsigned long long int value) -> gradian_f;
    inline auto operator""_grad(long double value) -> gradian_f;

}

}

#include "AngleUnits.inl"
