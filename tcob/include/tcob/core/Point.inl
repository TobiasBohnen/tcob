// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Point.hpp"

#include <cmath>

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
auto constexpr point<T>::dot(point<T> const& p) const -> float_type
{
    return static_cast<float_type>(X * p.X + Y * p.Y);
}

template <Arithmetic T>
auto constexpr point<T>::cross(point<T> const& p) const -> float_type
{
    return static_cast<float_type>(X * p.Y - Y * p.Y);
}

template <Arithmetic T>
auto constexpr point<T>::perpendicular() const -> point<T>
{
    return {-Y, X};
}

template <Arithmetic T>
inline auto point<T>::length() const -> float_type
{
    return static_cast<float_type>(std::sqrt((X * X) + (Y * Y)));
}

template <Arithmetic T>
inline auto point<T>::distance_to(point<T> const& p) const -> float_type
{
    return (*this - p).length();
}

template <Arithmetic T>
inline auto point<T>::angle_to(point<T> const& p) const -> degree<float_type>
{
    degree<float_type> retValue {radian<float_type> {static_cast<float_type>(std::atan2(p.Y - Y, p.X - X))}};
    retValue += degree<float_type> {90};
    if (retValue.Value < 0) { retValue += degree<float_type> {360}; }
    return retValue;
}

template <Arithmetic T>
inline auto point<T>::as_normalized() const -> point<float_type>
{
    auto const l {static_cast<float_type>(length())};
    if (l != 0) { return {static_cast<float_type>(X) / l, static_cast<float_type>(Y) / l}; }

    return {static_cast<float_type>(X), static_cast<float_type>(Y)};
}

template <Arithmetic T>
auto constexpr point<T>::Lerp(point<T> const& left, point<T> const& right, f64 step) -> point<T>
{
    T const x {static_cast<T>(left.X + ((right.X - left.X) * step))};
    T const y {static_cast<T>(left.Y + ((right.Y - left.Y) * step))};
    return {x, y};
}

template <Arithmetic T>
auto constexpr point<T>::FromDirection(degree<float_type> angle) -> point<T>
{
    radian<float_type> const rad {angle - degree<float_type> {90}};
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
}
