// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <ostream>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Concepts.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <Arithmetic T>
class [[nodiscard]] point final {
public:
    using type       = T;
    using float_type = f64;

    constexpr point() = default;
    constexpr point(T x, T y);

    template <typename U>
    explicit constexpr point(point<U> const& p);

    auto constexpr as_array [[nodiscard]] () const -> std::array<T, 2>;

    auto constexpr dot(point<T> const& p) const -> float_type;
    auto constexpr cross(point<T> const& p) const -> float_type;

    auto length() const -> float_type;
    auto distance_to(point<T> const& p) const -> float_type;

    auto angle_to(point<T> const& p) const -> degree<float_type>;

    auto as_normalized() const -> point<float_type>;

    auto constexpr equals(point<T> const& other, T tol) const -> bool;

    auto static constexpr Lerp(point<T> const& left, point<T> const& right, f64 step) -> point<T>;
    auto static constexpr FromDirection(degree<float_type> angle) -> point<T>;

    T X {0};
    T Y {0};

    static point<T> const Zero;
    static point<T> const One;
};

template <Arithmetic T>
point<T> const point<T>::Zero = {0, 0};

template <Arithmetic T>
point<T> const point<T>::One = {1, 1};

using point_i = point<i32>;
using point_u = point<u32>;
using point_f = point<f32>;
using point_d = point<f64>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator+=(point<T>& left, point<R> const& right) -> point<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator+(point<T> const& left, point<R> const& right) -> point<T>;

template <Arithmetic T>
auto constexpr operator-(point<T> const& right) -> point<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator-=(point<T>& left, point<R> const& right) -> point<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator-(point<T> const& left, point<R> const& right) -> point<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(point<T> const& left, point<R> const& right) -> point<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(point<T> const& left, R const& right) -> point<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator*=(point<T>& left, R right) -> point<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator*=(point<T>& left, point<R> const& right) -> point<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(point<T> const& left, point<R> const& right) -> point<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(point<T> const& left, R right) -> point<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator/=(point<T>& left, R right) -> point<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator/=(point<T>& left, point<R> const& right) -> point<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator==(point<T> const& left, point<R> const& right) -> bool;

template <Arithmetic T>
inline auto operator<<(std::ostream& os, point<T> const& m) -> std::ostream&;

template <Arithmetic T>
void Serialize(point<T> const& v, auto&& s)
{
    s["x"] = v.X;
    s["y"] = v.Y;
}

template <Arithmetic T>
auto Deserialize(point<T>& v, auto&& s) -> bool
{
    return s.try_get(v.X, "x") && s.try_get(v.Y, "y");
}

}

#include "Point.inl"

template <tcob::Arithmetic T>
struct std::formatter<tcob::point<T>> {
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(tcob::point<T> val, format_context& ctx) const
    {
        return format_to(ctx.out(), "(x:{},y:{})", val.X, val.Y);
    }
};

template <tcob::Arithmetic T>
struct std::hash<tcob::point<T>> {
    auto operator()(tcob::point<T> const& s) const -> std::size_t
    {
        std::size_t const h1 {std::hash<T> {}(s.X)};
        std::size_t const h2 {std::hash<T> {}(s.Y)};
        return tcob::helper::hash_combine(h1, h2);
    }
};
