// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <ostream>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Concepts.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <Arithmetic T>
class [[nodiscard]] point final {
public:
    using type       = T;
    using float_type = f64;

    constexpr point() = default;
    constexpr point(T x, T y);

    template <typename U>
    explicit constexpr point(point<U> const& p);

    auto constexpr as_array [[nodiscard]] () const -> std::array<T, 2>;

    auto length() const -> float_type;
    auto dot(point<T> const& p) const -> float_type;
    auto cross(point<T> const& p) const -> float_type;

    auto distance_to(point<T> const& p) const -> float_type;
    auto angle_to(point<T> const& p) const -> degree<float_type>;
    auto angle_between(point<T> const& p) const -> radian<float_type>;
    auto as_normalized() const -> point<float_type>;

    auto constexpr equals(point<T> const& other, T tol) const -> bool;

    auto static constexpr Lerp(point<T> const& left, point<T> const& right, f64 step) -> point<T>;

    void static Serialize(point const& v, auto&& s);
    auto static Deserialize(point& v, auto&& s) -> bool;

    T X {0};
    T Y {0};

    static point<T> const Zero;
    static point<T> const One;
};

template <Arithmetic T>
point<T> const point<T>::Zero = {0, 0};

template <Arithmetic T>
point<T> const point<T>::One = {1, 1};

using point_i = point<i32>;
using point_u = point<u32>;
using point_f = point<f32>;
using point_d = point<f64>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator+=(point<T>& left, point<R> const& right) -> point<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator+(point<T> const& left, point<R> const& right) -> point<T>;

template <Arithmetic T>
auto constexpr operator-(point<T> const& right) -> point<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator-=(point<T>& left, point<R> const& right) -> point<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator-(point<T> const& left, point<R> const& right) -> point<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(point<T> const& left, point<R> const& right) -> point<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(point<T> const& left, R const& right) -> point<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator*=(point<T>& left, R right) -> point<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator*=(point<T>& left, point<R> const& right) -> point<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(point<T> const& left, point<R> const& right) -> point<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(point<T> const& left, R right) -> point<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator/=(point<T>& left, R right) -> point<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator/=(point<T>& left, point<R> const& right) -> point<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator==(point<T> const& left, point<R> const& right) -> bool;

template <Arithmetic T>
inline auto operator<<(std::ostream& os, point<T> const& m) -> std::ostream&;

}

#include "Point.inl"
