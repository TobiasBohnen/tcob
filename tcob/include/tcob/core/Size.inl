// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Size.hpp"

#include <array>
#include <cmath>
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
auto constexpr size<T>::area() const -> T
{
    return Width * Height;
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
auto constexpr size<T>::Lerp(size const& from, size const& to, f64 step) -> size<T>
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
template <Arithmetic U>
auto constexpr size<T>::equals(size<U> const& other, f32 tol) const -> bool
{
    f32 const dw {static_cast<f32>(other.Width) - static_cast<f32>(Width)};
    f32 const dh {static_cast<f32>(other.Height) - static_cast<f32>(Height)};
    return std::abs(dw) <= tol && std::abs(dh) <= tol;
}

template <Arithmetic T>
template <Arithmetic U>
auto constexpr size<T>::as_fitted [[nodiscard]] (size<U> const& s) const -> size<T>
{
    f32 const factor {((static_cast<f32>(s.Width) / static_cast<f32>(s.Height)) > (static_cast<f32>(Width) / static_cast<f32>(Height)))
                          ? (static_cast<f32>(Width) / static_cast<f32>(s.Width))
                          : (static_cast<f32>(Height) / static_cast<f32>(s.Height))};
    return {static_cast<T>(static_cast<f32>(s.Width) * factor), static_cast<T>(static_cast<f32>(s.Height) * factor)};
}

template <Arithmetic T>
auto constexpr operator-(size<T> const& right) -> size<T>
{
    return size<T> {-right.Width, -right.Height};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator+=(size<T>& left, size<R> const& right) -> size<T>&
{
    left.Width += static_cast<T>(right.Width);
    left.Height += static_cast<T>(right.Height);

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator-=(size<T>& left, size<R> const& right) -> size<T>&
{
    left.Width -= static_cast<T>(right.Width);
    left.Height -= static_cast<T>(right.Height);

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator+(size<T> const& left, size<R> const& right) -> size<T>
{
    return {left.Width + static_cast<T>(right.Width), left.Height + static_cast<T>(right.Height)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator-(size<T> const& left, size<R> const& right) -> size<T>
{
    return {left.Width - static_cast<T>(right.Width), left.Height - static_cast<T>(right.Height)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(size<T> const& left, size<R> const& right) -> size<T>
{
    return {left.Width * static_cast<T>(right.Width), left.Height * static_cast<T>(right.Height)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(size<T> const& left, R right) -> size<T>
{
    return {left.Width * static_cast<T>(right), left.Height * static_cast<T>(right)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(R left, size<T> const& right) -> size<T>
{
    return right * left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*=(size<T>& left, R right) -> size<T>&
{
    left.Width *= static_cast<T>(right);
    left.Height *= static_cast<T>(right);

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*=(size<T>& left, size<R> const& right) -> size<T>&
{
    left.Width *= static_cast<T>(right.Width);
    left.Height *= static_cast<T>(right.Height);

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(size<T> const& left, size<R> const& right) -> size<T>
{
    return {left.Width / static_cast<T>(right.Width), left.Height / static_cast<T>(right.Height)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(size<T> const& left, R right) -> size<T>
{
    return {left.Width / static_cast<T>(right), left.Height / static_cast<T>(right)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/=(size<T>& left, R right) -> size<T>&
{
    left.Width /= static_cast<T>(right);
    left.Height /= static_cast<T>(right);

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/=(size<T>& left, size<R> const& right) -> size<T>&
{
    left.Width /= static_cast<T>(right.Width);
    left.Height /= static_cast<T>(right.Height);

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
        member<&size<T>::Width> {"width", "w"},
        member<&size<T>::Height> {"height", "h"}};
}
}
