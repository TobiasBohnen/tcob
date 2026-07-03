// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <cstddef>
#include <format>
#include <functional>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Concepts.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <Arithmetic T>
class [[nodiscard]] point final {
public:
    using type = T;

    constexpr point() = default;
    constexpr point(T x, T y);

    template <typename U>
    explicit constexpr point(point<U> const& p);

    auto constexpr to_array [[nodiscard]] () const -> std::array<T, 2>;

    template <Arithmetic U = T>
    auto constexpr dot(point<U> const& p) const -> f64;
    template <Arithmetic U = T>
    auto constexpr cross(point<U> const& p) const -> f64;

    auto length() const -> f64;
    auto distance_to(point const& p) const -> f64;

    auto angle_to(point const& p) const -> degree_d;
    auto moved_along(degree_d angle, T distance) const -> point<T>;

    auto constexpr as_perpendicular() const -> point;
    auto as_normalized() const -> point<f64>;

    template <Arithmetic U = T>
    auto constexpr equals(point<U> const& other, T tol) const -> bool;

    static auto constexpr Lerp(point const& from, point const& to, f64 step) -> point;
    static auto constexpr FromDirection(degree_d angle) -> point;

    T X {0};
    T Y {0};

    static point const Zero;
    static point const One;

    static auto constexpr Members();
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
auto constexpr operator*(point<T> const& left, R right) -> point<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(R left, point<T> const& right) -> point<T>;

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
auto euclidean_distance(point<T> const& a, point<T> const& b) -> f64;

template <Arithmetic T>
auto manhattan_distance(point<T> const& a, point<T> const& b) -> T;

template <Arithmetic T>
auto chebyshev_distance(point<T> const& a, point<T> const& b) -> T;

template <Arithmetic T>
auto minkowski_distance(point<T> const& a, point<T> const& b, f64 p) -> f64;

void bresenham_line(point_i from, point_i to, auto&& fn);

void bresenham_circle(point_i center, i32 radius, auto&& fn);
}

#include "Point.inl"

template <tcob::Arithmetic T>
struct std::formatter<tcob::point<T>> {
    auto constexpr parse(format_parse_context& ctx) { return ctx.begin(); }
    auto format(tcob::point<T> val, format_context& ctx) const { return format_to(ctx.out(), "(x:{},y:{})", val.X, val.Y); }
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
