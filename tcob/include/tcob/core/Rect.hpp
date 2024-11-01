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
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <Arithmetic T>
class [[nodiscard]] rect final {
public:
    using type = T;

    constexpr rect() = default;
    constexpr rect(point<T> p, size<T> s);
    constexpr rect(T left, T top, T width, T height);

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

    auto constexpr width() const -> T;
    auto constexpr height() const -> T;

    auto constexpr center() const -> point_f;
    auto constexpr local_center() const -> point_f;

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

    auto constexpr as_centered_at(point<T> const& center) const -> rect<T>;
    auto constexpr as_intersection_with(rect const& other) const -> rect;
    auto constexpr as_union_with(rect const& other) const -> rect;
    auto constexpr as_padded_by(size<T> const& size) const -> rect;

    auto static constexpr FromLTRB(T left, T top, T right, T bottom) -> rect<T>;

    auto static constexpr Lerp(rect const& left, rect const& right, f64 step) -> rect;

    point<T> Position;
    size<T>  Size;

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

template <Arithmetic T>
void Serialize(rect<T> const& v, auto&& s)
{
    s["x"]      = v.left();
    s["y"]      = v.top();
    s["width"]  = v.width();
    s["height"] = v.height();
}

template <Arithmetic T>
auto Deserialize(rect<T>& v, auto&& s) -> bool
{
    T x, y, w, h;
    if (s.try_get(x, "x") && s.try_get(y, "y") && s.try_get(w, "width") && s.try_get(h, "height")) {
        v.Position = {x, y};
        v.Size     = {w, h};
        return true;
    }
    return false;
}

}

#include "Rect.inl"

template <tcob::Arithmetic T>
struct std::formatter<tcob::rect<T>> {
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(tcob::rect<T> val, format_context& ctx) const
    {
        return format_to(ctx.out(), "(x:{},y:{},w:{},h:{})", val.left(), val.top(), val.width(), val.height());
    }
};

template <tcob::Arithmetic T>
struct std::hash<tcob::rect<T>> {
    auto operator()(tcob::rect<T> const& r) const noexcept -> std::size_t
    {
        std::size_t const h1 {std::hash<tcob::point<T>> {}(r.Position)};
        std::size_t const h2 {std::hash<tcob::size<T>> {}(r.Size)};
        return tcob::helper::hash_combine(h1, h2);
    }
};
