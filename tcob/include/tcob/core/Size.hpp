// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <ostream>

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

    template <Arithmetic U>
    auto constexpr contains(point<U> const& point) const -> bool;

    auto constexpr equals(size<T> const& other, f32 tol) const -> bool;

    auto static constexpr Lerp(size<T> const& left, size<T> const& right, f64 step) -> size<T>;

    void static Serialize(size const& v, auto&& s);
    auto static Deserialize(size& v, auto&& s) -> bool;

    T Width {0};
    T Height {0};

    static size<T> const Zero;
    static size<T> const One;
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
