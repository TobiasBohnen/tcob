// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Concepts.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

enum class angle_normalize : u8 {
    FullTurnSymmetric, // [-OneTurn, OneTurn)
    HalfTurnSymmetric, // [-HalfTurn, +HalfTurn)
    PositiveFullTurn   // [0, OneTurn)
};

////////////////////////////////////////////////////////////

template <FloatingPoint ValueType, f64 OneTurn>
class [[nodiscard]] angle_unit {
public:
    using value_type = ValueType;
    static constexpr value_type one_turn {static_cast<ValueType>(OneTurn)};

    constexpr angle_unit() = default;
    constexpr angle_unit(ValueType value);

    template <FloatingPoint ValueType2, f64 OneTurn2>
    constexpr angle_unit(angle_unit<ValueType2, OneTurn2> const& other) noexcept;

    auto sin [[nodiscard]] () const -> ValueType;
    auto asin [[nodiscard]] () const -> ValueType;
    auto cos [[nodiscard]] () const -> ValueType;
    auto acos [[nodiscard]] () const -> ValueType;
    auto tan [[nodiscard]] () const -> ValueType;
    auto atan [[nodiscard]] () const -> ValueType;

    auto constexpr as_normalized [[nodiscard]] (angle_normalize mode = angle_normalize::FullTurnSymmetric) const -> angle_unit<ValueType, OneTurn>;

    auto static constexpr Lerp(angle_unit const& left, angle_unit const& right, f64 step) -> angle_unit<ValueType, OneTurn>;

    ValueType Value {0};
};

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr operator+(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>;

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr operator+=(angle_unit<ValueType, OneTurn>& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>&;

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr operator-(angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>;

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr operator-(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>;

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr operator-=(angle_unit<ValueType, OneTurn>& left, angle_unit<ValueType, OneTurn> const& right) -> angle_unit<ValueType, OneTurn>&;

template <FloatingPoint ValueType, f64 OneTurn, Arithmetic R>
auto constexpr operator*(angle_unit<ValueType, OneTurn> const& left, R const& right) -> angle_unit<ValueType, OneTurn>;

template <FloatingPoint ValueType, f64 OneTurn, Arithmetic R>
auto constexpr operator*=(angle_unit<ValueType, OneTurn>& left, R right) -> angle_unit<ValueType, OneTurn>&;

template <FloatingPoint ValueType, f64 OneTurn, Arithmetic R>
auto constexpr operator/(angle_unit<ValueType, OneTurn> const& left, R right) -> angle_unit<ValueType, OneTurn>;

template <FloatingPoint ValueType, f64 OneTurn, Arithmetic R>
auto constexpr operator/=(angle_unit<ValueType, OneTurn>& left, R right) -> angle_unit<ValueType, OneTurn>&;

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr operator==(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> bool;

template <FloatingPoint ValueType, f64 OneTurn, Arithmetic R>
auto constexpr operator==(angle_unit<ValueType, OneTurn> const& left, R right) -> bool;

template <FloatingPoint ValueType, f64 OneTurn>
auto constexpr operator<=>(angle_unit<ValueType, OneTurn> const& left, angle_unit<ValueType, OneTurn> const& right) -> std::partial_ordering;

template <FloatingPoint ValueType, f64 OneTurn, Arithmetic R>
auto constexpr operator<=>(angle_unit<ValueType, OneTurn> const& left, R right) -> std::partial_ordering;

template <FloatingPoint ValueType, f64 OneTurn>
inline auto operator<<(std::ostream& os, angle_unit<ValueType, OneTurn> const& m) -> std::ostream&;

////////////////////////////////////////////////////////////

template <typename T>
using degree   = angle_unit<T, 360.>;
using degree_d = degree<f64>;
using degree_f = degree<f32>;

////////////////////////////////////////////////////////////

template <typename T>
using radian   = angle_unit<T, TAU>;
using radian_d = radian<f64>;
using radian_f = radian<f32>;

////////////////////////////////////////////////////////////

template <typename T>
using turn   = angle_unit<T, 1.>;
using turn_d = turn<f64>;
using turn_f = turn<f32>;

////////////////////////////////////////////////////////////

template <typename T>
using gradian   = angle_unit<T, 400.>;
using gradian_d = gradian<f64>;
using gradian_f = gradian<f32>;

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
