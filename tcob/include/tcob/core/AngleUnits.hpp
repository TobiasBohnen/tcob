// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Concepts.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <template <typename> typename Type, typename ValueType>
class [[nodiscard]] angle_unit {
public:
    using value_type = ValueType;

    constexpr angle_unit() = default;
    constexpr angle_unit(value_type value);

    template <template <typename> typename Type2>
    constexpr angle_unit(angle_unit<Type2, ValueType> const& other) noexcept;
    template <template <typename> typename Type2, typename ValueType2>
    explicit constexpr angle_unit(angle_unit<Type2, ValueType2> const& other) noexcept;

    auto sin [[nodiscard]] () const -> value_type;
    auto asin [[nodiscard]] () const -> value_type;
    auto cos [[nodiscard]] () const -> value_type;
    auto acos [[nodiscard]] () const -> value_type;
    auto tan [[nodiscard]] () const -> value_type;
    auto atan [[nodiscard]] () const -> value_type;

    auto constexpr as_normalized [[nodiscard]] () -> Type<ValueType>;

    auto static constexpr Lerp(angle_unit const& left, angle_unit const& right, f64 step) -> Type<ValueType>;

    value_type Value {0};
};

template <template <typename> typename Type, typename ValueType>
auto constexpr operator+(angle_unit<Type, ValueType> const& left, angle_unit<Type, ValueType> const& right) -> Type<ValueType>;

template <template <typename> typename Type, typename ValueType>
auto constexpr operator+=(angle_unit<Type, ValueType>& left, angle_unit<Type, ValueType> const& right) -> angle_unit<Type, ValueType>&;

template <template <typename> typename Type, typename ValueType>
auto constexpr operator-(angle_unit<Type, ValueType> const& right) -> Type<ValueType>;

template <template <typename> typename Type, typename ValueType>
auto constexpr operator-(angle_unit<Type, ValueType> const& left, angle_unit<Type, ValueType> const& right) -> Type<ValueType>;

template <template <typename> typename Type, typename ValueType>
auto constexpr operator-=(angle_unit<Type, ValueType>& left, angle_unit<Type, ValueType> const& right) -> angle_unit<Type, ValueType>&;

template <template <typename> typename Type, typename ValueType, Arithmetic R>
auto constexpr operator*(angle_unit<Type, ValueType> const& left, R const& right) -> Type<ValueType>;

template <template <typename> typename Type, typename ValueType, Arithmetic R>
auto constexpr operator*=(angle_unit<Type, ValueType>& left, R right) -> angle_unit<Type, ValueType>&;

template <template <typename> typename Type, typename ValueType, Arithmetic R>
auto constexpr operator/(angle_unit<Type, ValueType> const& left, R right) -> Type<ValueType>;

template <template <typename> typename Type, typename ValueType, Arithmetic R>
auto constexpr operator/=(angle_unit<Type, ValueType>& left, R right) -> angle_unit<Type, ValueType>&;

template <template <typename> typename Type, typename ValueType>
auto constexpr operator==(angle_unit<Type, ValueType> const& left, angle_unit<Type, ValueType> const& right) -> bool;

template <template <typename> typename Type, typename ValueType, Arithmetic R>
auto constexpr operator==(angle_unit<Type, ValueType> const& left, R right) -> bool;

template <template <typename> typename Type, typename ValueType>
auto constexpr operator<=>(angle_unit<Type, ValueType> const& left, angle_unit<Type, ValueType> const& right) -> std::partial_ordering;

template <template <typename> typename Type, typename ValueType, Arithmetic R>
auto constexpr operator<=>(angle_unit<Type, ValueType> const& left, R right) -> std::partial_ordering;

template <template <typename> typename Type, typename ValueType>
inline auto operator<<(std::ostream& os, angle_unit<Type, ValueType> const& m) -> std::ostream&;

////////////////////////////////////////////////////////////

template <FloatingPoint T>
class [[nodiscard]] degree final : public angle_unit<degree, T> {
public:
    using angle_unit<degree, T>::angle_unit;

    static constexpr T OneTurn {static_cast<T>(360)};
};

using degree_f = degree<f32>;
using degree_d = degree<f64>;

////////////////////////////////////////////////////////////

template <FloatingPoint T>
class [[nodiscard]] radian final : public angle_unit<radian, T> {
public:
    using angle_unit<radian, T>::angle_unit;

    static constexpr T OneTurn {static_cast<T>(TAU)};
};

using radian_f = radian<f32>;
using radian_d = radian<f64>;

////////////////////////////////////////////////////////////

template <FloatingPoint T>
class [[nodiscard]] turn final : public angle_unit<turn, T> {
public:
    using angle_unit<turn, T>::angle_unit;

    static constexpr T OneTurn {static_cast<T>(1)};
};

using turn_f = turn<f32>;
using turn_d = turn<f64>;

////////////////////////////////////////////////////////////

template <FloatingPoint T>
class [[nodiscard]] gradian final : public angle_unit<gradian, T> {
public:
    using angle_unit<gradian, T>::angle_unit;

    static constexpr T OneTurn {static_cast<T>(400)};
};

using gradian_f = gradian<f32>;
using gradian_d = gradian<f64>;

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
