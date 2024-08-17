// Copyright (c) 2024 Tobias Bohnen
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
auto constexpr point<T>::as_array [[nodiscard]] () const -> std::array<T, 2>
{
    return {X, Y};
}

template <Arithmetic T>
inline auto point<T>::length() const -> f32
{
    return static_cast<f32>(std::sqrt((X * X) + (Y * Y)));
}

template <Arithmetic T>
inline auto point<T>::distance_to(point<T> const& p) const -> f32
{
    return (*this - p).length();
}

template <Arithmetic T>
inline auto point<T>::as_normalized() const -> point<f32>
{
    f32 const l {length()};
    if (l != 0) { return {X / l, Y / l}; }

    return *this;
}

template <Arithmetic T>
auto constexpr point<T>::Lerp(point<T> const& left, point<T> const& right, f64 step) -> point<T>
{
    T const x {static_cast<T>(left.X + ((right.X - left.X) * step))};
    T const y {static_cast<T>(left.Y + ((right.Y - left.Y) * step))};
    return {x, y};
}

template <Arithmetic T>
auto constexpr point<T>::equals(point<T> const& other, T tol) const -> bool
{
    T const dx {other.X - X};
    T const dy {other.Y - Y};
    return dx * dx + dy * dy <= tol * tol;
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
    return {left.X + right.X, left.Y + right.Y};
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
    return {left.X - right.X, left.Y - right.Y};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(point<T> const& left, point<R> const& right) -> point<T>
{
    return {left.X * right.X, left.Y * right.Y};
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
    return {left.X / right.X, left.Y / right.Y};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(point<T> const& left, R const right) -> point<T>
{
    return {left.X / right, left.Y / right};
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
