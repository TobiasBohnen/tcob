// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Point.hpp"

#include <array>
#include <cmath>
#include <ostream>

#include "tcob/core/AngleUnits.hpp"

namespace tcob {

template <Arithmetic T>
constexpr point<T>::point(T x, T y)
    : X {x}
    , Y {y}
{
}

template <Arithmetic T>
template <typename U>
constexpr point<T>::point(point<U> const& p)
    : X {static_cast<T>(p.X)}
    , Y {static_cast<T>(p.Y)}
{
}

template <Arithmetic T>
auto constexpr point<T>::to_array [[nodiscard]] () const -> std::array<T, 2>
{
    return {X, Y};
}

template <Arithmetic T>
auto constexpr point<T>::dot(point<T> const& p) const -> f64
{
    return static_cast<f64>(X * p.X + Y * p.Y);
}

template <Arithmetic T>
auto constexpr point<T>::cross(point<T> const& p) const -> f64
{
    return static_cast<f64>(X * p.Y - Y * p.Y);
}

template <Arithmetic T>
inline auto point<T>::length() const -> f64
{
    return static_cast<f64>(std::sqrt((X * X) + (Y * Y)));
}

template <Arithmetic T>
inline auto point<T>::distance_to(point<T> const& p) const -> f64
{
    return euclidean_distance(*this, p);
}

template <Arithmetic T>
inline auto point<T>::angle_to(point<T> const& p) const -> degree_d
{
    degree_d retValue {radian_d {static_cast<f64>(std::atan2(p.Y - Y, p.X - X))}};
    retValue += degree_d {90};
    if (retValue.Value < 0) { retValue += degree_d {360}; }
    return retValue;
}

template <Arithmetic T>
auto constexpr point<T>::as_perpendicular() const -> point<T>
{
    return {-Y, X};
}

template <Arithmetic T>
inline auto point<T>::as_normalized() const -> point<f64>
{
    auto const l {static_cast<f64>(length())};
    if (l != 0) { return {static_cast<f64>(X) / l, static_cast<f64>(Y) / l}; }

    return {static_cast<f64>(X), static_cast<f64>(Y)};
}

template <Arithmetic T>
auto constexpr point<T>::Lerp(point<T> const& left, point<T> const& right, f64 step) -> point<T>
{
    T const x {static_cast<T>(left.X + ((right.X - left.X) * step))};
    T const y {static_cast<T>(left.Y + ((right.Y - left.Y) * step))};
    return {x, y};
}

template <Arithmetic T>
auto constexpr point<T>::FromDirection(degree_d angle) -> point<T>
{
    radian_d const rad {angle - degree_d {90}};
    return point<T> {point_d {rad.cos(), rad.sin()}.as_normalized()};
}

template <Arithmetic T>
auto constexpr point<T>::equals(point<T> const& other, T tol) const -> bool
{
    T const dx {other.X - X};
    T const dy {other.Y - Y};
    return dx * dx + dy * dy <= tol * tol;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator+=(point<T>& left, point<R> const& right) -> point<T>&
{
    left.X += right.X;
    left.Y += right.Y;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator+(point<T> const& left, point<R> const& right) -> point<T>
{
    return {static_cast<T>(left.X + right.X), static_cast<T>(left.Y + right.Y)};
}

template <Arithmetic T>
auto constexpr operator-(point<T> const& right) -> point<T>
{
    return point<T> {-right.X, -right.Y};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator-=(point<T>& left, point<R> const& right) -> point<T>&
{
    left.X -= right.X;
    left.Y -= right.Y;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator-(point<T> const& left, point<R> const& right) -> point<T>
{
    return {static_cast<T>(left.X - right.X), static_cast<T>(left.Y - right.Y)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(point<T> const& left, point<R> const& right) -> point<T>
{
    return {static_cast<T>(left.X * right.X), static_cast<T>(left.Y * right.Y)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(point<T> const& left, R const& right) -> point<T>
{
    return {static_cast<T>(left.X * right), static_cast<T>(left.Y * right)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*=(point<T>& left, R const right) -> point<T>&
{
    left.X *= right;
    left.Y *= right;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*=(point<T>& left, point<R> const& right) -> point<T>&
{
    left.X *= right.X;
    left.Y *= right.Y;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(point<T> const& left, point<R> const& right) -> point<T>
{
    return {static_cast<T>(left.X / right.X), static_cast<T>(left.Y / right.Y)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(point<T> const& left, R const right) -> point<T>
{
    return {static_cast<T>(left.X / right), static_cast<T>(left.Y / right)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/=(point<T>& left, R const right) -> point<T>&
{
    left.X /= right;
    left.Y /= right;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/=(point<T>& left, point<R> const& right) -> point<T>&
{
    left.X /= right.X;
    left.Y /= right.Y;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator==(point<T> const& left, point<R> const& right) -> bool
{
    return (left.X == right.X) && (left.Y == right.Y);
}

template <Arithmetic T>
inline auto operator<<(std::ostream& os, point<T> const& m) -> std::ostream&
{
    return os << "x:" << m.X << "|y:" << m.Y;
}

template <Arithmetic T>
inline void point<T>::Serialize(point<T> const& v, auto&& s)
{
    s["x"] = v.X;
    s["y"] = v.Y;
}

template <Arithmetic T>
inline auto point<T>::Deserialize(point<T>& v, auto&& s) -> bool
{
    return s.try_get(v.X, "x") && s.try_get(v.Y, "y");
}

template <Arithmetic T>
auto euclidean_distance(point<T> const& a, point<T> const& b) -> f64
{
    return (a - b).length();
}

template <Arithmetic T>
auto manhattan_distance(point<T> const& a, point<T> const& b) -> T
{
    return std::abs(a.X - b.X) + std::abs(a.Y - b.Y);
}

template <Arithmetic T>
auto chebyshev_distance(point<T> const& a, point<T> const& b) -> T
{
    return std::max(std::abs(a.X - b.X), std::abs(a.Y - b.Y));
}

template <Arithmetic T>
auto minkowski_distance(point<T> const& a, point<T> const& b, f64 p) -> f64
{
    return std::pow(std::pow(std::abs(a.X - b.X), p) + std::pow(std::abs(a.Y - b.Y), p), 1.0 / p);
}

}
