// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Size.hpp"

#include <array>
#include <numeric>
#include <tuple>

#include "tcob/core/Point.hpp"
#include "tcob/core/Serialization.hpp"

namespace tcob {

template <Arithmetic T>
constexpr size<T>::size(T width, T height)
    : Width {width}
    , Height {height}
{
}

template <Arithmetic T>
template <typename U>
constexpr size<T>::size(size<U> const& p)
    : Width {static_cast<T>(p.Width)}
    , Height {static_cast<T>(p.Height)}
{
}

template <Arithmetic T>
auto constexpr size<T>::aspect_ratio() const -> f32
{
    if (Width == 0 || Height == 0) { return 0; }
    return static_cast<f32>(Width) / static_cast<f32>(Height);
}

template <Arithmetic T>
auto constexpr size<T>::integer_ratio() const -> size<i32>
{
    if (Width == 0 || Height == 0) { return {0, 0}; }
    auto const divisor {std::gcd(static_cast<i32>(Width), static_cast<i32>(Height))};
    return {static_cast<i32>(Width / divisor), static_cast<i32>(Height / divisor)};
}

template <Arithmetic T>
auto constexpr size<T>::to_array [[nodiscard]] () const -> std::array<T, 2>
{
    return {Width, Height};
}

template <Arithmetic T>
auto constexpr size<T>::Lerp(size<T> const& from, size<T> const& to, f64 step) -> size<T>
{
    T const w {helper::lerp(from.Width, to.Width, step)};
    T const h {helper::lerp(from.Height, to.Height, step)};
    return {w, h};
}

template <Arithmetic T>
template <Arithmetic U>
auto constexpr size<T>::contains(point<U> const& point) const -> bool
{
    return (point.X >= 0) && (point.X < Width) && (point.Y >= 0) && (point.Y < Height);
}

template <Arithmetic T>
auto constexpr size<T>::equals(size<T> const& other, f32 tol) const -> bool
{
    f32 const dw {static_cast<f32>(other.Width) - Width};
    f32 const dh {static_cast<f32>(other.Height) - Height};
    return (dw * dw) + (dh * dh) <= tol * tol;
}

template <Arithmetic T>
auto constexpr size<T>::as_fitted [[nodiscard]] (size<T> const& s) const -> size<T>
{
    f32 const factor {((s.Width / s.Height) > (Width / Height)) ? (Width / s.Width) : (Height / s.Height)};
    return {s.Width * factor, s.Height * factor};
}

template <Arithmetic T>
auto constexpr operator-(size<T> const& right) -> size<T>
{
    return size<T> {-right.Width, -right.Height};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator+=(size<T>& left, size<R> const& right) -> size<T>&
{
    left.Width += right.Width;
    left.Height += right.Height;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator-=(size<T>& left, size<R> const& right) -> size<T>&
{
    left.Width -= right.Width;
    left.Height -= right.Height;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator+(size<T> const& left, size<R> const& right) -> size<T>
{
    return {static_cast<T>(left.Width + right.Width), static_cast<T>(left.Height + right.Height)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator-(size<T> const& left, size<R> const& right) -> size<T>
{
    return {static_cast<T>(left.Width - right.Width), static_cast<T>(left.Height - right.Height)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(size<T> const& left, size<R> const& right) -> size<T>
{
    return {static_cast<T>(left.Width * right.Width), static_cast<T>(left.Height * right.Height)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(size<T> const& left, R const right) -> size<T>
{
    return {static_cast<T>(left.Width * right), static_cast<T>(left.Height * right)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(T const left, size<R> const& right) -> size<T>
{
    return {static_cast<T>(right.Width * left), static_cast<T>(right.Height * left)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*=(size<T>& left, R const right) -> size<T>&
{
    left.Width *= right;
    left.Height *= right;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*=(size<T>& left, size<R> const& right) -> size<T>&
{
    left.Width *= right.Width;
    left.Height *= right.Height;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(size<T> const& left, size<R> const& right) -> size<T>
{
    return {static_cast<T>(left.Width / right.Width), static_cast<T>(left.Height / right.Height)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(size<T> const& left, R const right) -> size<T>
{
    return {static_cast<T>(left.Width / right), static_cast<T>(left.Height / right)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/=(size<T>& left, R const right) -> size<T>&
{
    left.Width /= right;
    left.Height /= right;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/=(size<T>& left, size<R> const& right) -> size<T>&
{
    left.Width /= right.Width;
    left.Height /= right.Height;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator==(size<T> const& left, size<R> const& right) -> bool
{
    return (left.Width == right.Width) && (left.Height == right.Height);
}

template <Arithmetic T>
auto constexpr size<T>::Members()
{
    return std::tuple {
        member<&size<T>::Width> {"width"},
        member<&size<T>::Height> {"height"}};
}
}
