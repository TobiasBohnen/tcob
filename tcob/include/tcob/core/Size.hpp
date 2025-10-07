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

#include "tcob/core/Common.hpp"
#include "tcob/core/Concepts.hpp"
#include "tcob/core/Point.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <Arithmetic T>
class [[nodiscard]] size final {
public:
    using type = T;

    constexpr size() = default;
    constexpr size(T width, T height);

    template <typename U>
    explicit constexpr size(size<U> const& p);

    auto constexpr area() const -> T;
    auto constexpr aspect_ratio() const -> f32;
    auto constexpr integer_ratio() const -> size<i32>;

    auto constexpr to_array [[nodiscard]] () const -> std::array<T, 2>;

    template <Arithmetic U = T>
    auto constexpr contains(point<U> const& point) const -> bool;

    auto constexpr equals(size<T> const& other, f32 tol) const -> bool;

    auto constexpr as_fitted(size<T> const& s) const -> size<T>;

    static auto constexpr Lerp(size<T> const& from, size<T> const& to, f64 step) -> size<T>;

    T Width {0};
    T Height {0};

    static size<T> const Zero;
    static size<T> const One;

    static auto constexpr Members();
};

template <Arithmetic T>
size<T> const size<T>::Zero = {0, 0};

template <Arithmetic T>
size<T> const size<T>::One = {1, 1};

using size_i = size<i32>;
using size_u = size<u32>;
using size_f = size<f32>;
using size_d = size<f64>;

template <Arithmetic T>
auto constexpr operator-(size<T> const& right) -> size<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator+=(size<T>& left, size<R> const& right) -> size<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator-=(size<T>& left, size<R> const& right) -> size<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator+(size<T> const& left, size<R> const& right) -> size<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator-(size<T> const& left, size<R> const& right) -> size<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(size<T> const& left, size<R> const& right) -> size<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(size<T> const& left, R right) -> size<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(T left, size<R> const& right) -> size<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator*=(size<T>& left, R right) -> size<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator*=(size<T>& left, size<R> const& right) -> size<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(size<T> const& left, size<R> const& right) -> size<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(size<T> const& left, R right) -> size<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator/=(size<T>& left, R right) -> size<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator/=(size<T>& left, size<R> const& right) -> size<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator==(size<T> const& left, size<R> const& right) -> bool;
}

#include "Size.inl"

template <tcob::Arithmetic T>
struct std::formatter<tcob::size<T>> {
    auto constexpr parse(format_parse_context& ctx) { return ctx.begin(); }
    auto format(tcob::size<T> val, format_context& ctx) const { return format_to(ctx.out(), "(w:{},h:{})", val.Width, val.Height); }
};

template <tcob::Arithmetic T>
struct std::hash<tcob::size<T>> {
    auto operator()(tcob::size<T> const& s) const -> std::size_t
    {
        std::size_t const h1 {std::hash<T> {}(s.Width)};
        std::size_t const h2 {std::hash<T> {}(s.Height)};
        return tcob::helper::hash_combine(h1, h2);
    }
};
