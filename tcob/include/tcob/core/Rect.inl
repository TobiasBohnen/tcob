// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Rect.hpp"

#include <algorithm>

namespace tcob {

template <Arithmetic T>
constexpr rect<T>::rect(T left, T top, T width, T height)
    : X {left}
    , Y {top}
    , Width {width}
    , Height {height}
{
}

template <Arithmetic T>
constexpr rect<T>::rect(point<T> tl, tcob::size<T> s)
    : X {tl.X}
    , Y {tl.Y}
    , Width {s.Width}
    , Height {s.Height}
{
}

template <Arithmetic T>
template <Arithmetic U>
constexpr rect<T>::rect(rect<U> const& p)
    : X {static_cast<T>(p.X)}
    , Y {static_cast<T>(p.Y)}
    , Width {static_cast<T>(p.Width)}
    , Height {static_cast<T>(p.Height)}
{
}

template <Arithmetic T>
auto constexpr rect<T>::FromLTRB(T left, T top, T right, T bottom) -> rect<T>
{
    return {left, top, right - left, bottom - top};
}

template <Arithmetic T>
auto constexpr rect<T>::intersects(rect const& rectangle) const -> bool
{
    std::pair<T, T> const r1MinMaxX {std::minmax(X, static_cast<T>(X + Width))};
    std::pair<T, T> const r2MinMaxX {std::minmax(rectangle.X, static_cast<T>(rectangle.X + rectangle.Width))};

    if (std::max(r1MinMaxX.first, r2MinMaxX.first) < std::min(r1MinMaxX.second, r2MinMaxX.second)) {
        std::pair<T, T> const r1MinMaxY {std::minmax(Y, static_cast<T>(Y + Height))};
        std::pair<T, T> const r2MinMaxY {std::minmax(rectangle.Y, static_cast<T>(rectangle.Y + rectangle.Height))};
        return std::max(r1MinMaxY.first, r2MinMaxY.first) < std::min(r1MinMaxY.second, r2MinMaxY.second);
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

    f32 const rectAtan {std::atan2(Height, Width)};
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

    point_f retValue {X + Width / 2, Y + Height / 2};
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
        retValue.X += xFactor * (Width / 2.0f);               // "Z0"
        retValue.Y += yFactor * (Width / 2.0f) * tanTheta;
    } else {
        retValue.X += xFactor * (Height / (2.0f * tanTheta)); // "Z1"
        retValue.Y += yFactor * (Height / 2.0f);
    }

    return retValue;
}

template <Arithmetic T>
auto constexpr rect<T>::as_intersection(rect const& other) const -> rect
{
    T const x1 {std::max(X, other.X)};
    T const y1 {std::max(Y, other.Y)};
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
auto constexpr rect<T>::as_shrunk(size<T> const& size) const -> rect
{
    T const x = X + static_cast<T>(size.Width / 2);
    T const y = Y + static_cast<T>(size.Height / 2);
    T const w = Width - static_cast<T>(size.Width);
    T const h = Height - static_cast<T>(size.Height);

    return {x, y, w, h};
}

template <Arithmetic T>
auto constexpr rect<T>::Lerp(rect const& left, rect const& right, f64 step) -> rect
{
    auto const pos {point<T>::Lerp(left.get_position(), right.get_position(), step)};
    auto const siz {size<T>::Lerp(left.get_size(), right.get_size(), step)};
    return {pos, siz};
}

template <Arithmetic T>
auto constexpr rect<T>::get_position() const -> point<T>
{
    return {X, Y};
}

template <Arithmetic T>
auto constexpr rect<T>::get_size() const -> size<T>
{
    return {Width, Height};
}

template <Arithmetic T>
auto constexpr rect<T>::left() const -> T
{
    return X;
}

template <Arithmetic T>
auto constexpr rect<T>::top() const -> T
{
    return Y;
}

template <Arithmetic T>
auto constexpr rect<T>::top_left() const -> point<T>
{
    return {X, Y};
}

template <Arithmetic T>
auto constexpr rect<T>::top_right() const -> point<T>
{
    return {X + Width, Y};
}

template <Arithmetic T>
auto constexpr rect<T>::bottom() const -> T
{
    return Y + Height;
}

template <Arithmetic T>
auto constexpr rect<T>::right() const -> T
{
    return X + Width;
}

template <Arithmetic T>
auto constexpr rect<T>::bottom_left() const -> point<T>
{
    return {X, Y + Height};
}

template <Arithmetic T>
auto constexpr rect<T>::bottom_right() const -> point<T>
{
    return {X + Width, Y + Height};
}

template <Arithmetic T>
auto constexpr rect<T>::get_center() const -> point_f
{
    return {static_cast<f32>(X) + (static_cast<f32>(Width) / 2), static_cast<f32>(Y) + (static_cast<f32>(Height) / 2)};
}

template <Arithmetic T>
auto constexpr rect<T>::get_local_center() const -> point_f
{
    return {static_cast<f32>(Width) / 2, static_cast<f32>(Height) / 2};
}

template <Arithmetic T>
template <Arithmetic U>
void constexpr rect<T>::move_by(point<U> const& point)
{
    X += static_cast<T>(point.X);
    Y += static_cast<T>(point.Y);
}

template <Arithmetic T>
template <Arithmetic U>
void constexpr rect<T>::resize_by(size<U> const& size)
{
    Width += static_cast<T>(size.Width);
    Height += static_cast<T>(size.Height);
}

template <Arithmetic T>
auto constexpr rect<T>::with_position(point<T> const& point) const -> rect<T>
{
    return {point, get_size()};
}

template <Arithmetic T>
auto constexpr rect<T>::with_size(size<T> const& size) const -> rect<T>
{
    return {get_position(), size};
}

template <Arithmetic T>
inline void rect<T>::Serialize(rect<T> const& v, auto&& s)
{
    s["x"]      = v.X;
    s["y"]      = v.Y;
    s["width"]  = v.Width;
    s["height"] = v.Height;
}

template <Arithmetic T>
inline auto rect<T>::Deserialize(rect<T>& v, auto&& s) -> bool
{
    return s.try_get(v.X, "x") && s.try_get(v.Y, "y")
        && s.try_get(v.Width, "width") && s.try_get(v.Height, "height");
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
    std::pair<T, T> const minMaxX {std::minmax(X, static_cast<T>(X + Width))};
    std::pair<T, T> const minMaxY {std::minmax(Y, static_cast<T>(Y + Height))};

    return (static_cast<T>(point.X) >= minMaxX.first) && (static_cast<T>(point.X) < minMaxX.second)
        && (static_cast<T>(point.Y) >= minMaxY.first) && (static_cast<T>(point.Y) < minMaxY.second);
}

template <Arithmetic T>
auto constexpr rect<T>::equals(rect<T> const& other, f32 tol) const -> bool
{
    f32 const dx {static_cast<f32>(other.X) - X};
    f32 const dy {static_cast<f32>(other.Y) - Y};
    f32 const dw {static_cast<f32>(other.Width) - Width};
    f32 const dh {static_cast<f32>(other.Height) - Height};
    return dx * dx + dy * dy + dw * dw + dh * dh <= tol * tol;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(rect<T> const& left, R const& right) -> rect<T>
{
    return {static_cast<T>(left.X * right), static_cast<T>(left.Y * right),
            static_cast<T>(left.Width * right), static_cast<T>(left.Height * right)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator*=(rect<T>& left, R const& right) -> rect<T>&
{
    left.X *= right;
    left.Y *= right;
    left.Width *= right;
    left.Height *= right;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(rect<T> const& left, R const& right) -> rect<T>
{
    return {static_cast<T>(left.X / right), static_cast<T>(left.Y / right),
            static_cast<T>(left.Width / right), static_cast<T>(left.Height / right)};
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator/=(rect<T>& left, R const& right) -> rect<T>&
{
    left.X /= right;
    left.Y /= right;
    left.Width /= right;
    left.Height /= right;

    return left;
}

template <Arithmetic T, Arithmetic R>
auto constexpr operator==(rect<T> const& left, rect<R> const& right) -> bool
{
    return (left.X == right.X) && (left.Y == right.Y)
        && (left.Width == right.Width) && (left.Height == right.Height);
}

template <Arithmetic T>
inline auto operator<<(std::ostream& os, rect<T> const& m) -> std::ostream&
{
    return os << "left:" << m.X << "|top:" << m.Y << "|width:" << m.Width << "|height:" << m.Height;
}
}
