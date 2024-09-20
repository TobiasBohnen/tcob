// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <ostream>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Concepts.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <Arithmetic T>
class [[nodiscard]] rect final {
public:
    using type = T;

    constexpr rect() = default;
    constexpr rect(T left, T top, T width, T height);
    constexpr rect(point<T> tl, size<T> s);

    template <Arithmetic U>
    explicit constexpr rect(rect<U> const& p);

    auto constexpr left() const -> T;
    auto constexpr top() const -> T;
    auto constexpr top_left() const -> point<T>;
    auto constexpr top_right() const -> point<T>;
    auto constexpr bottom() const -> T;
    auto constexpr right() const -> T;
    auto constexpr bottom_left() const -> point<T>;
    auto constexpr bottom_right() const -> point<T>;

    auto constexpr get_position() const -> point<T>;
    auto constexpr get_size() const -> size<T>;
    auto constexpr get_center() const -> point_f;
    auto constexpr get_local_center() const -> point_f;

    template <Arithmetic U>
    void constexpr move_by(point<U> const& point);
    template <Arithmetic U>
    void constexpr resize_by(size<U> const& size);

    template <Arithmetic U>
    auto constexpr contains [[nodiscard]] (point<U> const& point) const -> bool;
    template <Arithmetic U>
    auto constexpr contains [[nodiscard]] (rect<U> const& rect) const -> bool;

    auto constexpr intersects [[nodiscard]] (rect const& rectangle) const -> bool;

    auto constexpr equals(rect<T> const& other, f32 tol) const -> bool;

    auto constexpr find_edge(degree_f angle) const -> point<T>;

    auto constexpr as_moved_to(point<T> const& point) const -> rect<T>;
    auto constexpr as_centered_at(point<T> const& center) const -> rect<T>;
    auto constexpr as_resized_to(size<T> const& size) const -> rect<T>;
    auto constexpr as_intersection_with(rect const& other) const -> rect;
    auto constexpr as_union_with(rect const& other) const -> rect;
    auto constexpr as_padded_by(size<T> const& size) const -> rect;

    auto static constexpr FromLTRB(T left, T top, T right, T bottom) -> rect<T>;

    auto static constexpr Lerp(rect const& left, rect const& right, f64 step) -> rect;

    void static Serialize(rect const& v, auto&& s);
    auto static Deserialize(rect& v, auto&& s) -> bool;

    T X {0};
    T Y {0};
    T Width {0};
    T Height {0};

    static rect<T> const Zero;
};

using rect_i = rect<i32>;
using rect_u = rect<u32>;
using rect_f = rect<f32>;
using rect_d = rect<f64>;

template <Arithmetic T>
rect<T> const rect<T>::Zero = {0, 0, 0, 0};

template <Arithmetic T, Arithmetic R>
auto constexpr operator*(rect<T> const& left, R const& right) -> rect<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator*=(rect<T>& left, R const& right) -> rect<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator/(rect<T> const& left, R const& right) -> rect<T>;

template <Arithmetic T, Arithmetic R>
auto constexpr operator/=(rect<T>& left, R const& right) -> rect<T>&;

template <Arithmetic T, Arithmetic R>
auto constexpr operator==(rect<T> const& left, rect<R> const& right) -> bool;

template <Arithmetic T>
inline auto operator<<(std::ostream& os, rect<T> const& m) -> std::ostream&;

}

#include "Rect.inl"
