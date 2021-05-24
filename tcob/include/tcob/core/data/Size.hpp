// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/core/Helper.hpp>

namespace tcob {
#pragma pack(push, 1)
template <Arithmetic T>
struct Size final {
    constexpr Size() = default;

    constexpr Size(T width, T height)
        : Width { width }
        , Height { height }
    {
    }

    template <typename U>
    explicit constexpr Size(const Size<U>& p)
        : Width { static_cast<T>(p.Width) }
        , Height { static_cast<T>(p.Height) }
    {
    }

    inline auto interpolate(const Size<T>& other, f64 step) const -> Size<T>
    {
        const T w { static_cast<T>(Width + ((other.Width - Width) * step)) };
        const T h { static_cast<T>(Height + ((other.Height - Height) * step)) };
        return { w, h };
    }

    T Width { 0 };
    T Height { 0 };

    static const Size<T> Zero;
    static const Size<T> One;
};
#pragma pack(pop)

template <Arithmetic T>
const Size<T> Size<T>::Zero;

template <Arithmetic T>
const Size<T> Size<T>::One { 1, 1 };

using SizeI = Size<i32>;
using SizeU = Size<u32>;
using SizeF = Size<f32>;

template <Arithmetic T>
inline constexpr auto operator-(const Size<T>& right) -> Size<T>
{
    return Size<T>(-right.Width, -right.Height);
}

template <Arithmetic T>
inline constexpr auto operator+=(Size<T>& left, const Size<T>& right) -> Size<T>&
{
    left.Width += right.Width;
    left.Height += right.Height;

    return left;
}

template <Arithmetic T>
inline constexpr auto operator-=(Size<T>& left, const Size<T>& right) -> Size<T>&
{
    left.Width -= right.Width;
    left.Height -= right.Height;

    return left;
}

template <Arithmetic T>
inline constexpr auto operator+(const Size<T>& left, const T& right) -> Size<T>
{
    return { left.Width + right, left.Height + right };
}

template <Arithmetic T>
inline constexpr auto operator-(const Size<T>& left, const T& right) -> Size<T>
{
    return { left.Width - right, left.Height - right };
}

template <Arithmetic T>
inline constexpr auto operator+(const Size<T>& left, const Size<T>& right) -> Size<T>
{
    return { left.Width + right.Width, left.Height + right.Height };
}

template <Arithmetic T>
inline constexpr auto operator-(const Size<T>& left, const Size<T>& right) -> Size<T>
{
    return { left.Width - right.Width, left.Height - right.Height };
}

template <Arithmetic T>
inline constexpr auto operator*(const Size<T>& left, const Size<T>& right) -> Size<T>
{
    return { left.Width * right.Width, left.Height * right.Height };
}

template <Arithmetic T>
inline constexpr auto operator*(const Size<T>& left, const T right) -> Size<T>
{
    return { left.Width * right, left.Height * right };
}

template <Arithmetic T>
inline constexpr auto operator*(const T left, const Size<T>& right) -> Size<T>
{
    return { right.Width * left, right.Height * left };
}

template <Arithmetic T>
inline constexpr auto operator*=(Size<T>& left, const T right) -> Size<T>&
{
    left.Width *= right;
    left.Height *= right;

    return left;
}

template <Arithmetic T>
inline constexpr auto operator*=(Size<T>& left, const Size<T>& right) -> Size<T>&
{
    left.Width *= right.Width;
    left.Height *= right.Height;

    return left;
}

template <Arithmetic T>
inline constexpr auto operator/(const Size<T>& left, const Size<T>& right) -> Size<T>
{
    return { left.Width / right.Width, left.Height / right.Height };
}

template <Arithmetic T>
inline constexpr auto operator/(const Size<T>& left, const T right) -> Size<T>
{
    return { left.Width / right, left.Height / right };
}

template <Arithmetic T>
inline constexpr auto operator/=(Size<T>& left, const T right) -> Size<T>&
{
    left.Width /= right;
    left.Height /= right;

    return left;
}

template <Arithmetic T>
inline constexpr auto operator/=(Size<T>& left, const Size<T>& right) -> Size<T>&
{
    left.Width /= right.Width;
    left.Height /= right.Height;

    return left;
}

template <Arithmetic T>
inline constexpr auto operator==(const Size<T>& left, const Size<T>& right) -> bool
{
    return (left.Width == right.Width) && (left.Height == right.Height);
}

template <Arithmetic T>
inline constexpr auto operator!=(const Size<T>& left, const Size<T>& right) -> bool
{
    return (left.Width != right.Width) || (left.Height != right.Height);
}

template <Arithmetic T>
inline auto operator<<(std::ostream& os, const Size<T>& m) -> std::ostream&
{
    return os << "width:" << m.Width << "|height:" << m.Height;
}
}