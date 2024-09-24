// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Ray.hpp"

namespace tcob::gfx {

f32 constexpr epsilon {std::numeric_limits<f32>::epsilon()};

ray::ray(point_f origin, degree_d direction)
    : Origin {origin}
    , Direction {direction}
{
}

auto ray::intersect_line(point_f a, point_f b) const -> std::optional<point_f>
{
    auto const r {get_direction()};
    if (!r) { return std::nullopt; }

    if (auto distance {intersect_segment(*r, point_d {a}, point_d {b})}) {
        return point_f {Origin + *r * *distance};
    }
    return std::nullopt;
}

auto ray::intersect_rect(rect_f const& rect, transform const& xform) const -> std::vector<point_f>
{
    auto const r {get_direction()};
    if (!r) { return {}; }

    point_d const topLeft {xform * rect.top_left()};
    point_d const topRight {xform * rect.top_right()};
    point_d const bottomLeft {xform * rect.bottom_left()};
    point_d const bottomRight {xform * rect.bottom_right()};

    std::vector<point_f> retValue;
    if (auto distance {intersect_segment(*r, topLeft, topRight)}) { retValue.emplace_back(Origin + *r * *distance); }
    if (auto distance {intersect_segment(*r, topRight, bottomRight)}) { retValue.emplace_back(Origin + *r * *distance); }
    if (auto distance {intersect_segment(*r, bottomRight, bottomLeft)}) { retValue.emplace_back(Origin + *r * *distance); }
    if (auto distance {intersect_segment(*r, bottomLeft, topLeft)}) { retValue.emplace_back(Origin + *r * *distance); }
    return retValue;
}

auto ray::intersect_circle(point_f const& center, f32 radius) -> std::vector<point_f>
{
    auto const r {get_direction()};
    if (!r) { return {}; }

    f64 const b {2.0 * ((Origin.X - center.X) * r->X + (Origin.Y - center.Y) * r->Y)};
    f64 const c {((Origin.X - center.X) * (Origin.X - center.X)) + ((Origin.Y - center.Y) * (Origin.Y - center.Y)) - (radius * radius)};

    f64 const discr {(b * b) - (4 * c)};
    if (discr < 0) { return {}; }

    f64 const sqrtDiscr {std::sqrt(discr)};
    f64 const s1 {(-b - sqrtDiscr) / 2};
    f64 const s2 {(-b + sqrtDiscr) / 2};

    std::vector<point_f> retValue;
    if (s1 >= 0) { retValue.emplace_back(Origin + *r * s1); }
    if (s2 >= 0 && s2 != s1) { retValue.emplace_back(Origin + *r * s2); }
    return retValue;
}

auto ray::intersect_segment(point_d const& rd, point_d const& p0, point_d const& p1) const -> std::optional<f64>
{
    point_d const ro {Origin};
    point_d const seg {p1 - p0};
    point_d const segPerp {seg.Y, -seg.X};

    f64 const denom {segPerp.dot(rd)};
    if (std::abs(denom) < epsilon) { return std::nullopt; }

    point_d const d {p0 - ro};
    f64 const     distance {segPerp.dot(d) / denom};
    f64 const     s {point_d {rd.Y, -rd.X}.dot(d) / denom};
    if (distance >= 0.0 && s >= 0.0 && s <= 1.0) { return distance; }

    return std::nullopt;
}

auto ray::get_direction() const -> std::optional<point_d>
{
    auto const r {point_d::FromDirection(Direction)};
    if (std::abs(r.X) < epsilon && std::abs(r.Y) < epsilon) {
        return std::nullopt; // Invalid ray direction
    }

    return r;
}
}
