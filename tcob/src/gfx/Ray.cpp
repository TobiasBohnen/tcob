// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Ray.hpp"

#include <cmath>
#include <cstdlib>
#include <limits>
#include <optional>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Polygon.hpp"

namespace tcob::gfx {

f32 constexpr epsilon {std::numeric_limits<f32>::epsilon()};

ray::ray(point_f origin, degree_f direction, f64 maxDistance)
    : _origin {origin}
    , _maxDistance {maxDistance}
    , _direction {point_d::FromDirection(direction)}
{
}

auto ray::get_point(f64 distance) const -> point_f
{
    return _origin + (_direction * distance);
}

void ray::intersect_line(point_f a, point_f b, intersect_callback const& fn) const
{
    if (auto const distance {intersect_segment(_direction, point_d {a}, point_d {b})}) {
        fn(get_result(*distance));
    }
}

void ray::intersect_rect(rect_f const& rect, intersect_callback const& fn) const
{
    intersect_rect(rect.top_left(), rect.top_right(), rect.bottom_left(), rect.bottom_right(), fn);
}

void ray::intersect_rect(point_f topLeft, point_f topRight, point_f bottomLeft, point_f bottomRight, intersect_callback const& fn) const
{
    if (auto const distance {intersect_segment(_direction, point_d {topLeft}, point_d {topRight})}) { fn(get_result(*distance)); }
    if (auto const distance {intersect_segment(_direction, point_d {topRight}, point_d {bottomRight})}) { fn(get_result(*distance)); }
    if (auto const distance {intersect_segment(_direction, point_d {bottomRight}, point_d {bottomLeft})}) { fn(get_result(*distance)); }
    if (auto const distance {intersect_segment(_direction, point_d {bottomLeft}, point_d {topLeft})}) { fn(get_result(*distance)); }
}

void ray::intersect_circle(point_f const& center, f64 radius, intersect_callback const& fn) const
{
    point_d const oc {_origin - center};
    f64 const     b {2.0 * oc.dot(_direction)};
    f64 const     c {oc.dot(oc) - (radius * radius)};

    f64 const discr {(b * b) - (4.0 * c)};
    if (discr < 0) { return; }

    f64 const sqrtDiscr {std::sqrt(discr)};
    f64 const s1 {(-b - sqrtDiscr) / 2};
    f64 const s2 {(-b + sqrtDiscr) / 2};

    if (s1 >= 0 && s1 <= _maxDistance) { fn(get_result(s1)); }
    if (s2 >= 0 && s2 != s1 && s2 <= _maxDistance) { fn(get_result(s2)); }
}

void ray::intersect_function(func const& func, f64 tolerance, intersect_callback const& fn) const
{
    point_f                lastPoint {func(0)};
    std::optional<point_f> lastHit;
    for (f64 t {0}; t <= 1; t += tolerance) {
        point_f const cp {func(t)};
        if (auto const distance {intersect_segment(_direction, point_d {lastPoint}, point_d {cp})}) {
            point_f const p {get_point(*distance)};
            if (p != lastHit) {
                fn(get_result(*distance));
                lastHit = p;
            }
        }
        lastPoint = cp;
    }
}

void ray::intersect_polyline(polyline_span polygon, intersect_callback const& fn) const
{
    usize const n {polygon.size()};
    for (usize i {0}; i < n; ++i) {
        if (auto const distance {intersect_segment(_direction, point_d {polygon[i]}, point_d {polygon[(i + 1) % n]})}) {
            fn(get_result(*distance));
        }
    }
}

auto ray::intersect_segment(point_d const& rd, point_d const& p0, point_d const& p1) const -> std::optional<f64>
{
    point_d const ro {_origin};
    point_d const seg {p1 - p0};
    point_d const segPerp {seg.as_perpendicular()};

    f64 const denom {segPerp.dot(rd)};
    if (std::abs(denom) < epsilon) { return std::nullopt; }

    point_d const d {p0 - ro};
    f64 const     distance {segPerp.dot(d) / denom};
    if (distance < 0.0 || distance > _maxDistance) { return std::nullopt; }

    f64 const s {rd.as_perpendicular().dot(d) / denom};
    if (s >= 0.0 && s <= 1.0) { return distance; }

    return std::nullopt;
}

auto ray::get_result(f64 distance) const -> result
{
    return {.Point = get_point(distance), .Distance = distance};
}

}
