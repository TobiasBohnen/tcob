// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <algorithm>

#include <tcob/core/Helper.hpp>
#include <tcob/core/data/Point.hpp>
#include <tcob/core/data/Size.hpp>

namespace tcob {
#pragma pack(push, 1)
template <Arithmetic T>
struct Rect final {
    constexpr Rect()
    {
    }

    constexpr Rect(T left, T top, T width, T height)
        : Left { left }
        , Top { top }
        , Width { width }
        , Height { height }
    {
    }

    constexpr Rect(Point<T> topLeft, Size<T> size)
        : Left { topLeft.X }
        , Top { topLeft.Y }
        , Width { size.Width }
        , Height { size.Height }
    {
    }

    template <typename U>
    explicit constexpr Rect(const Rect<U>& p)
        : Left { static_cast<T>(p.Left) }
        , Top { static_cast<T>(p.Top) }
        , Width { static_cast<T>(p.Width) }
        , Height { static_cast<T>(p.Height) }
    {
    }

    static constexpr auto FromLTRB(T left, T top, T right, T bottom) -> Rect<T>
    {
        return { left, top, right - left, bottom - top };
    }

    inline auto contains(const Point<T>& point) const -> bool
    {
        const std::pair<T, T> minMaxX { std::minmax(Left, static_cast<T>(Left + Width)) };
        const std::pair<T, T> minMaxY { std::minmax(Top, static_cast<T>(Top + Height)) };

        return (point.X >= minMaxX.first) && ((point.X) < minMaxX.second) && (point.Y >= minMaxY.first) && ((point.Y) < minMaxY.second);
    }

    inline auto contains(const Rect<T>& rect) const -> bool
    {
        return contains(rect.top_left()) && contains(rect.bottom_right());
    }

    inline auto intersects(const Rect<T>& rectangle) const -> bool
    {
        const std::pair<T, T> r1MinMaxX { std::minmax(Left, static_cast<T>(Left + Width)) };
        const std::pair<T, T> r2MinMaxX { std::minmax(rectangle.Left, static_cast<T>(rectangle.Left + rectangle.Width)) };

        if (std::max(r1MinMaxX.first, r2MinMaxX.first) < std::min(r1MinMaxX.second, r2MinMaxX.second)) {
            const std::pair<T, T> r1MinMaxY { std::minmax(Top, static_cast<T>(Top + Height)) };
            const std::pair<T, T> r2MinMaxY { std::minmax(rectangle.Top, static_cast<T>(rectangle.Top + rectangle.Height)) };
            return std::max(r1MinMaxY.first, r2MinMaxY.first) < std::min(r1MinMaxY.second, r2MinMaxY.second);
        } else {
            return false;
        }
    }

    inline auto position() const -> Point<T>
    {
        return { Left, Top };
    }

    inline void position(const Point<T>& pos)
    {
        Left = pos.X;
        Top = pos.Y;
    }

    inline auto size() const -> Size<T>
    {
        return { Width, Height };
    }

    inline void size(const Size<T>& size)
    {
        Width = size.Width;
        Height = size.Height;
    }

    inline auto top_left() const -> Point<T>
    {
        return { Left, Top };
    }

    inline auto top_right() const -> Point<T>
    {
        return { Left + Width, Top };
    }

    inline auto bottom() const -> T
    {
        return Top + Height;
    }

    inline auto right() const -> T
    {
        return Left + Width;
    }

    inline auto bottom_left() const -> Point<T>
    {
        return { Left, Top + Height };
    }

    inline auto bottom_right() const -> Point<T>
    {
        return { Left + Width, Top + Height };
    }

    inline auto center() const -> PointF
    {
        return { Left + (static_cast<f32>(Width) / 2), Top + (static_cast<f32>(Height) / 2) };
    }

    inline auto center_local() const -> PointF
    {
        return { static_cast<f32>(Width) / 2, static_cast<f32>(Height) / 2 };
    }

    inline auto interpolate(const Rect<T>& other, f64 step) const -> Rect<T>
    {
        const auto pos { position().interpolate(other.position(), step) };
        const auto siz { size().interpolate(other.size(), step) };
        return { pos, siz };
    }

    T Left { 0 };
    T Top { 0 };
    T Width { 0 };
    T Height { 0 };

    static const Rect<T> Zero;
};
#pragma pack(pop)

using RectI = Rect<i32>;
using RectU = Rect<u32>;
using RectF = Rect<f32>;

template <Arithmetic T>
const Rect<T> Rect<T>::Zero;

template <Arithmetic T>
inline constexpr auto operator==(const Rect<T>& left, const Rect<T>& right) -> bool
{
    return (left.Left == right.Left) && (left.Top == right.Top)
        && (left.Width == right.Width) && (left.Height == right.Height);
}

template <Arithmetic T>
inline constexpr auto operator!=(const Rect<T>& left, const Rect<T>& right) -> bool
{
    return (left.Left != right.Left) || (left.Top != right.Top)
        || (left.Width != right.Width) || (left.Height != right.Height);
}

template <Arithmetic T>
inline constexpr auto operator*(const Rect<T>& left, const Size<T>& right) -> Rect<T>
{
    return { left.Left * right.Width, left.Top * right.Height, left.Width * right.Width, left.Height * right.Height };
}

template <Arithmetic T>
inline constexpr auto operator*=(Rect<T>& left, const Size<T>& right) -> Rect<T>&
{
    left.Left *= right.Width;
    left.Top *= right.Height;
    left.Width *= right.Width;
    left.Height *= right.Height;

    return left;
}

template <Arithmetic T>
inline constexpr auto operator/(const Rect<T>& left, const Size<T>& right) -> Rect<T>
{
    return { left.Left / right.Width, left.Top / right.Height, left.Width / right.Width, left.Height / right.Height };
}

template <Arithmetic T>
inline constexpr auto operator/=(Rect<T>& left, const Size<T>& right) -> Rect<T>&
{
    left.Left /= right.Width;
    left.Top /= right.Height;
    left.Width /= right.Width;
    left.Height /= right.Height;

    return left;
}

template <Arithmetic T>
inline auto operator<<(std::ostream& os, const Rect<T>& m) -> std::ostream&
{
    return os << "left:" << m.Left << "|top:" << m.Top << "|width:" << m.Width << "|height:" << m.Height;
}
}