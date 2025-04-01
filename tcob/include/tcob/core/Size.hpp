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
#include <ostream>

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

    auto constexpr to_array [[nodiscard]] () const -> std::array<T, 2>;

    template <Arithmetic U>
    auto constexpr contains(point<U> const& point) const -> bool;

    auto constexpr equals(size<T> const& other, f32 tol) const -> bool;

    auto constexpr as_fitted(size<T> const& s) const -> size<T>;

    auto static constexpr Lerp(size<T> const& left, size<T> const& right, f64 step) -> size<T>;

    T Width {0};
    T Height {0};

    static size<T> const Zero;
    static size<T> const One;

    void static Serialize(size<T> const& v, auto&& s)
    {
        s["width"]  = v.Width;
        s["height"] = v.Height;
    }

    auto static Deserialize(size<T>& v, auto&& s) -> bool
    {
        return s.try_get(v.Width, "width") && s.try_get(v.Height, "height");
    }
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

template <Arithmetic T>
inline auto operator<<(std::ostream& os, size<T> const& m) -> std::ostream&;

}

#include "Size.inl"

template <tcob::Arithmetic T>
struct std::formatter<tcob::size<T>> {
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(tcob::size<T> val, format_context& ctx) const
    {
        return format_to(ctx.out(), "(w:{},h:{})", val.Width, val.Height);
    }
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
