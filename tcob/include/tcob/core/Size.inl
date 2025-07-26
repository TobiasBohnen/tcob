// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Size.hpp"

#include <array>
#include <ostream>

#include "tcob/core/Point.hpp"

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
auto constexpr size<T>::to_array [[nodiscard]] () const -> std::array<T, 2>
{
    return {Width, Height};
}

template <Arithmetic T>
auto constexpr size<T>::Lerp(size<T> const& left, size<T> const& right, f64 step) -> size<T>
{
    T const w {static_cast<T>(left.Width + ((right.Width - left.Width) * step))};
    T const h {static_cast<T>(left.Height + ((right.Height - left.Height) * step))};
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
    return dw * dw + dh * dh <= tol * tol;
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
inline auto operator<<(std::ostream& os, size<T> const& m) -> std::ostream&
{
    return os << "width:" << m.Width << "|height:" << m.Height;
}

template <Arithmetic T>
inline void size<T>::Serialize(size<T> const& v, auto&& s)
{
    s["width"]  = v.Width;
    s["height"] = v.Height;
}

template <Arithmetic T>
inline auto size<T>::Deserialize(size<T>& v, auto&& s) -> bool
{
    return s.try_get(v.Width, "width") && s.try_get(v.Height, "height");
}

}
