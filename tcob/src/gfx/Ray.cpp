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

auto static IntersectsSegment(point_d const& ro, point_d const& rd, point_d const& p0, point_d const& p1) -> std::optional<f64>
{
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

auto ray::intersect_line(point_f a, point_f b) const -> std::optional<point_f>
{
    auto const r {get_direction()};
    if (!r) { return std::nullopt; }

    if (auto distance {IntersectsSegment(point_d {Origin}, *r, point_d {a}, point_d {b})}) {
        return point_f {Origin + *r * *distance};
    }
    return std::nullopt;
}

auto ray::intersect_rect(rect_f const& rect) const -> std::vector<point_f>
{
    std::vector<point_f> retValue;

    point_f const topLeft {rect.top_left()};
    point_f const topRight {rect.top_right()};
    point_f const bottomLeft {rect.bottom_left()};
    point_f const bottomRight {rect.bottom_right()};

    if (auto intersection {intersect_line(topLeft, topRight)}) { retValue.push_back(*intersection); }
    if (auto intersection {intersect_line(topRight, bottomRight)}) { retValue.push_back(*intersection); }
    if (auto intersection {intersect_line(bottomRight, bottomLeft)}) { retValue.push_back(*intersection); }
    if (auto intersection {intersect_line(bottomLeft, topLeft)}) { retValue.push_back(*intersection); }

    return retValue;
}

auto ray::intersect_polygon(polygon_span polygon) const -> std::vector<point_f>
{
    auto const r {get_direction()};
    if (!r) { return {}; }

    std::vector<point_f> retValue;

    usize const n {polygon.size()};
    for (usize i {0}; i < n; ++i) {
        if (auto distance {IntersectsSegment(point_d {Origin}, *r, point_d {polygon[i]}, point_d {polygon[(i + 1) % n]})}) {
            retValue.emplace_back(Origin + *r * *distance);
        }
    }

    return retValue;
}

auto ray::get_direction() const -> std::optional<point_d>
{
    radian_d const rad {Direction - degree_d {90}};
    point_d        r {rad.cos(), rad.sin()};
    r = r.as_normalized();
    if (std::abs(r.X) < epsilon && std::abs(r.Y) < epsilon) {
        return std::nullopt; // Invalid ray direction
    }

    return r;
}
}
