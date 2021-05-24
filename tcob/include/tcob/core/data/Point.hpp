// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <cmath>

#include <tcob/core/Helper.hpp>

namespace tcob {

#pragma pack(push, 1)
template <Arithmetic T>
struct Point final {
    constexpr Point() = default;

    constexpr Point(T x, T y)
        : X { x }
        , Y { y }
    {
    }

    template <typename U>
    explicit constexpr Point(const Point<U>& p)
        : X { static_cast<T>(p.X) }
        , Y { static_cast<T>(p.Y) }
    {
    }

    inline auto length() const -> T
    {
        return static_cast<T>(std::sqrt((X * X) + (Y * Y)));
    }

    inline auto distance(const Point<T>& p) const -> T
    {
        return (*this - p).length();
    }

    inline auto normalized() const -> Point<f32>
    {
        const f32 l { static_cast<f32>(length()) };
        if (l != 0)
            return { X / l, Y / l };
        else
            return *this;
    }

    inline auto equals(const Point<T>& other, f32 tol) const -> bool
    {
        const f32 dx { other.X - X };
        const f32 dy { other.Y - Y };
        return dx * dx + dy * dy < tol * tol;
    }

    inline auto interpolate(const Point<T>& other, f64 step) const -> Point<T>
    {
        const T x { static_cast<T>(X + ((other.X - X) * step)) };
        const T y { static_cast<T>(Y + ((other.Y - Y) * step)) };
        return { x, y };
    }

    T X { 0 };
    T Y { 0 };

    static const Point<T> Zero;
    static const Point<T> One;
};
#pragma pack(pop)

template <Arithmetic T>
const Point<T> Point<T>::Zero;

template <Arithmetic T>
const Point<T> Point<T>::One { 1, 1 };

using PointI = Point<i32>;
using PointU = Point<u32>;
using PointF = Point<f32>;

template <Arithmetic T>
inline constexpr auto operator+=(Point<T>& left, const Point<T>& right) -> Point<T>&
{
    left.X += right.X;
    left.Y += right.Y;

    return left;
}

template <Arithmetic T>
inline constexpr auto operator+(const Point<T>& left, const Point<T>& right) -> Point<T>
{
    return { left.X + right.X, left.Y + right.Y };
}

template <Arithmetic T, Arithmetic R>
inline constexpr auto operator+(const Point<T>& left, const R& right) -> Point<T>
{
    return { static_cast<T>(left.X + right), static_cast<T>(left.Y + right) };
}

template <Arithmetic T>
inline constexpr auto operator-(const Point<T>& right) -> Point<T>
{
    return Point<T>(-right.X, -right.Y);
}

template <Arithmetic T>
inline constexpr auto operator-=(Point<T>& left, const Point<T>& right) -> Point<T>&
{
    left.X -= right.X;
    left.Y -= right.Y;

    return left;
}

template <Arithmetic T>
inline constexpr auto operator-(const Point<T>& left, const Point<T>& right) -> Point<T>
{
    return { left.X - right.X, left.Y - right.Y };
}

template <Arithmetic T, Arithmetic R>
inline constexpr auto operator-(const Point<T>& left, const R& right) -> Point<T>
{
    return { static_cast<T>(left.X - right), static_cast<T>(left.Y - right) };
}

template <Arithmetic T>
inline constexpr auto operator*(const Point<T>& left, const Point<T>& right) -> Point<T>
{
    return { left.X * right.X, left.Y * right.Y };
}

template <Arithmetic T, Arithmetic R>
inline constexpr auto operator*(const Point<T>& left, const R& right) -> Point<T>
{
    return { static_cast<T>(left.X * right), static_cast<T>(left.Y * right) };
}

template <Arithmetic T>
inline constexpr auto operator*=(Point<T>& left, const T right) -> Point<T>&
{
    left.X *= right;
    left.Y *= right;

    return left;
}

template <Arithmetic T>
inline constexpr auto operator*=(Point<T>& left, const Point<T>& right) -> Point<T>&
{
    left.X *= right.X;
    left.Y *= right.Y;

    return left;
}

template <Arithmetic T>
inline constexpr auto operator/(const Point<T>& left, const Point<T>& right) -> Point<T>
{
    return { left.X / right.X, left.Y / right.Y };
}

template <Arithmetic T>
inline constexpr auto operator/(const Point<T>& left, const T right) -> Point<T>
{
    return { left.X / right, left.Y / right };
}

template <Arithmetic T>
inline constexpr auto operator/=(Point<T>& left, const T right) -> Point<T>&
{
    left.X /= right;
    left.Y /= right;

    return left;
}

template <Arithmetic T>
inline constexpr auto operator/=(Point<T>& left, const Point<T>& right) -> Point<T>&
{
    left.X /= right.X;
    left.Y /= right.Y;

    return left;
}

template <Arithmetic T>
inline constexpr auto operator==(const Point<T>& left, const Point<T>& right) -> bool
{
    return (left.X == right.X) && (left.Y == right.Y);
}

template <Arithmetic T>
inline constexpr auto operator!=(const Point<T>& left, const Point<T>& right) -> bool
{
    return (left.X != right.X) || (left.Y != right.Y);
}

template <Arithmetic T>
inline auto operator<<(std::ostream& os, const Point<T>& m) -> std::ostream&
{
    return os << "x:" << m.X << "|y:" << m.Y;
}

}