// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Rect.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <ostream>
#include <utility>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob {

template <Arithmetic T>
constexpr rect<T>::rect(point<T> p, size<T> s)
    : Position {p}
    , Size(s)
{
}

template <Arithmetic T>
constexpr rect<T>::rect(T left, T top, T width, T height)
    : rect {{left, top}, {width, height}}
{
}

template <Arithmetic T>
template <Arithmetic U>
constexpr rect<T>::rect(rect<U> const& p)
    : rect {static_cast<point<T>>(p.Position), static_cast<size<T>>(p.Size)}
{
}

template <Arithmetic T>
auto constexpr rect<T>::FromLTRB(T left, T top, T right, T bottom) -> rect<T>
{
    return {left, top, right - left, bottom - top};
}

template <Arithmetic T>
auto constexpr rect<T>::intersects(rect const& rectangle, bool includeEdges) const -> bool
{
    std::pair<T, T> const r1MinMaxX {std::minmax(Position.X, static_cast<T>(Position.X + Size.Width))};
    std::pair<T, T> const r2MinMaxX {std::minmax(rectangle.Position.X, static_cast<T>(rectangle.Position.X + rectangle.Size.Width))};

    if (includeEdges) {
        if (std::max(r1MinMaxX.first, r2MinMaxX.first) <= std::min(r1MinMaxX.second, r2MinMaxX.second)) {
            std::pair<T, T> const r1MinMaxY {std::minmax(Position.Y, static_cast<T>(Position.Y + Size.Height))};
            std::pair<T, T> const r2MinMaxY {std::minmax(rectangle.Position.Y, static_cast<T>(rectangle.Position.Y + rectangle.Size.Height))};
            return std::max(r1MinMaxY.first, r2MinMaxY.first) <= std::min(r1MinMaxY.second, r2MinMaxY.second);
        }
    } else {
        if (std::max(r1MinMaxX.first, r2MinMaxX.first) < std::min(r1MinMaxX.second, r2MinMaxX.second)) {
            std::pair<T, T> const r1MinMaxY {std::minmax(Position.Y, static_cast<T>(Position.Y + Size.Height))};
            std::pair<T, T> const r2MinMaxY {std::minmax(rectangle.Position.Y, static_cast<T>(rectangle.Position.Y + rectangle.Size.Height))};
            return std::max(r1MinMaxY.first, r2MinMaxY.first) < std::min(r1MinMaxY.second, r2MinMaxY.second);
        }
    }

    return false;
}

template <Arithmetic T>
auto constexpr rect<T>::find_edge(degree_f angle) const -> point<T>
{
    // Ref: http://stackoverflow.com/questions/4061576/finding-points-on-a-rectangle-at-a-given-angle
    f32 theta {radian_f {degree_f {360} - angle.as_normalized()}.Value};

    while (theta < -std::numbers::pi_v<f32>) { theta += TAU_F; }
    while (theta > std::numbers::pi_v<f32>) { theta -= TAU_F; }

    f32 const rectAtan {std::atan2(Size.Height, Size.Width)};
    f32 const tanTheta {std::tan(theta)};
    i32       region {};

    if ((theta > -rectAtan) && (theta <= rectAtan)) {
        region = 1;
    } else if ((theta > rectAtan) && (theta <= (std::numbers::pi_v<f32> - rectAtan))) {
        region = 2;
    } else if ((theta > (std::numbers::pi_v<f32> - rectAtan)) || (theta <= -(std::numbers::pi_v<f32> - rectAtan))) {
        region = 3;
    } else {
        region = 4;
    }

    point_f retValue {Position.X + Size.Width / 2, Position.Y + Size.Height / 2};
    f32     xFactor {1};
    f32     yFactor {1};

    switch (region) {
    case 1:
    case 2:
        yFactor = -1;
        break;
    case 3:
    case 4:
        xFactor = -1;
        break;
    }

    if (region == 1 || region == 3) {
        retValue.X += xFactor * (Size.Width / 2.0f);               // "Z0"
        retValue.Y += yFactor * (Size.Width / 2.0f) * tanTheta;
    } else {
        retValue.X += xFactor * (Size.Height / (2.0f * tanTheta)); // "Z1"
        retValue.Y += yFactor * (Size.Height / 2.0f);
    }

    return retValue;
}

template <Arithmetic T>
auto constexpr rect<T>::as_centered_at(point<T> const& center) const -> rect
{
    return {{center.X - Size.Width / 2.0f, center.Y - Size.Height / 2.0f}, Size};
}

template <Arithmetic T>
auto constexpr rect<T>::as_intersection_with(rect const& other) const -> rect
{
    T const x1 {std::max(Position.X, other.Position.X)};
    T const y1 {std::max(Position.Y, other.Position.Y)};
    T const x2 {std::min(right(), other.right())};
    T const y2 {std::min(bottom(), other.bottom())};

    T const width {x2 - x1};
    T const height {y2 - y1};

    if (width > 0 && height > 0) {
        return {x1, y1, width, height};
    }

    return {0, 0, 0, 0};
}

template <Arithmetic T>
auto constexpr rect<T>::as_union_with(rect const& other) const -> rect
{
    T const x1 {std::min(Position.X, other.Position.X)};
    T const y1 {std::min(Position.Y, other.Position.Y)};
    T const x2 {std::max(right(), other.right())};
    T const y2 {std::max(bottom(), other.bottom())};

    return {x1, y1, x2 - x1, y2 - y1};
}

template <Arithmetic T>
auto constexpr rect<T>::as_padded_by(size<T> const& size) const -> rect
{
    T const x {Position.X + static_cast<T>(size.Width / 2)};
    T const y {Position.Y + static_cast<T>(size.Height / 2)};
    T const w {Size.Width - static_cast<T>(size.Width)};
    T const h {Size.Height - static_cast<T>(size.Height)};

    return {x, y, w, h};
}

template <Arithmetic T>
auto constexpr rect<T>::Lerp(rect const& left, rect const& right, f64 step) -> rect
{
    auto const pos {point<T>::Lerp(left.Position, right.Position, step)};
    auto const siz {size<T>::Lerp(left.Size, right.Size, step)};
    return {pos, siz};
}

template <Arithmetic T>
auto constexpr rect<T>::left() const -> T
{
    return Position.X;
}

template <Arithmetic T>
auto constexpr rect<T>::top() const -> T
{
    return Position.Y;
}

template <Arithmetic T>
auto constexpr rect<T>::top_left() const -> point<T>
{
    return {Position.X, Position.Y};
}

template <Arithmetic T>
auto constexpr rect<T>::top_right() const -> point<T>
{
    return {Position.X + Size.Width, Position.Y};
}

template <Arithmetic T>
auto constexpr rect<T>::bottom() const -> T
{
    return Position.Y + Size.Height;
}

template <Arithmetic T>
auto constexpr rect<T>::right() const -> T
{
    return Position.X + Size.Width;
}

template <Arithmetic T>
auto constexpr rect<T>::bottom_left() const -> point<T>
{
    return {Position.X, Position.Y + Size.Height};
}

template <Arithmetic T>
auto constexpr rect<T>::bottom_right() const -> point<T>
{
    return {Position.X + Size.Width, Position.Y + Size.Height};
}

template <Arithmetic T>
auto constexpr rect<T>::width() const -> T
{
    return Size.Width;
}

template <Arithmetic T>
auto constexpr rect<T>::height() const -> T
{
    return Size.Height;
}

template <Arithmetic T>
auto constexpr rect<T>::center() const -> point_f
{
    return {static_cast<f32>(Position.X) + (static_cast<f32>(Size.Width) / 2), static_cast<f32>(Position.Y) + (static_cast<f32>(Size.Height) / 2)};
}

template <Arithmetic T>
auto constexpr rect<T>::local_center() const -> point_f
{
    return {static_cast<f32>(Size.Width) / 2, static_cast<f32>(Size.Height) / 2};
}

template <Arithmetic T>
template <Arithmetic U>
void constexpr rect<T>::move_by(point<U> const& point)
{
    Position += point;
}

template <Arithmetic T>
template <Arithmetic U>
void constexpr rect<T>::resize_by(size<U> const& size)
{
    Size += size;
}

template <Arithmetic T>
template <Arithmetic U>
auto constexpr rect<T>::contains(rect<U> const& rect) const -> bool
{
    return contains(rect.top_left()) && contains(rect.bottom_right());
}

template <Arithmetic T>
template <Arithmetic U>
auto constexpr rect<T>::contains(point<U> const& point) const -> bool
{
    std::pair<T, T> const minMaxX {std::minmax(Position.X, static_cast<T>(Position.X + Size.Width))};
    std::pair<T, T> const minMaxY {std::minmax(Position.Y, static_cast<T>(Position.Y + Size.Height))};

    return (static_cast<T>(point.X) >= minMaxX.first) && (static_cast<T>(point.X) < minMaxX.second)
        && (static_cast<T>(point.Y) >= minMaxY.first) && (static_cast<T>(point.Y) < minMaxY.second);
}

template <Arithmetic T>
auto constexpr rect<T>::equals(rect const& other, f32 tol) const -> bool
{
    f32 const dx {static_cast<f32>(other.Position.X) - Position.X};
    f32 const dy {static_cast<f32>(other.Position.Y) - Position.Y};
    f32 const dw {static_cast<f32>(other.Size.Width) - Size.Width};
    f32 const dh {static_cast<f32>(other.Size.Height) - Size.Height};
    return (dx * dx + dy * dy + dw * dw + dh * dh) <= (tol * tol);
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(rect<T> const& left, R const& right) -> rect<T>
{
    return {static_cast<T>(left.Position.X * right), static_cast<T>(left.Position.Y * right),
            static_cast<T>(left.Size.Width * right), static_cast<T>(left.Size.Height * right)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*=(rect<T>& left, R const& right) -> rect<T>&
{
    left.Position *= right;
    left.Size *= right;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(rect<T> const& left, R const& right) -> rect<T>
{
    return {static_cast<T>(left.Position.X / right), static_cast<T>(left.Position.Y / right),
            static_cast<T>(left.Size.Width / right), static_cast<T>(left.Size.Height / right)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/=(rect<T>& left, R const& right) -> rect<T>&
{
    left.Position /= right;
    left.Size /= right;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator==(rect<T> const& left, rect<R> const& right) -> bool
{
    return (left.Position == right.Position) && (left.Size == right.Size);
}

template <Arithmetic T>
inline auto operator<<(std::ostream& os, rect<T> const& m) -> std::ostream&
{
    return os << "left:" << m.left() << "|top:" << m.top() << "|width:" << m.width() << "|height:" << m.height();
}
}
