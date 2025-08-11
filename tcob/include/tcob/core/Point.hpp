// Copyright (c) 2025 Tobias Bohnen
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

    auto constexpr dot(point<T> const& p) const -> f64;
    auto constexpr cross(point<T> const& p) const -> f64;

    auto length() const -> f64;
    auto distance_to(point<T> const& p) const -> f64;

    auto angle_to(point<T> const& p) const -> degree<f64>;

    auto constexpr as_perpendicular() const -> point<T>;
    auto as_normalized() const -> point<f64>;

    auto constexpr equals(point<T> const& other, T tol) const -> bool;

    auto static constexpr Lerp(point<T> const& left, point<T> const& right, f64 step) -> point<T>;
    auto static constexpr FromDirection(degree<f64> angle) -> point<T>;

    T X {0};
    T Y {0};

    static point<T> const Zero;
    static point<T> const One;

    void static Serialize(point<T> const& v, auto&& s);
    auto static Deserialize(point<T>& v, auto&& s) -> bool;
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
auto euclidean_distance(point<T> const& a, point<T> const& b) -> f64;

template <Arithmetic T>
auto manhattan_distance(point<T> const& a, point<T> const& b) -> T;

template <Arithmetic T>
auto chebyshev_distance(point<T> const& a, point<T> const& b) -> T;

template <Arithmetic T>
auto minkowski_distance(point<T> const& a, point<T> const& b, f64 p) -> f64;

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
