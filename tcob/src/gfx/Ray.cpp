// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Ray.hpp"

namespace tcob::gfx {

f32 constexpr epsilon {std::numeric_limits<f32>::epsilon()};

ray::ray(point_f origin, degree_d direction)
    : _origin {origin}
    , _angle {direction}
    , _direction {point_d::FromDirection(direction)}
{
}

auto ray::get_origin() const -> point_f
{
    return _origin;
}

auto ray::get_direction() const -> degree_d
{
    return _angle;
}

auto ray::get_direction_vector() const -> point_d
{
    return _direction;
}

auto ray::intersect_line(point_f a, point_f b) const -> std::optional<result>
{
    if (auto distance {intersect_segment(_direction, point_d {a}, point_d {b})}) {
        return result {.Point = point_f {_origin + _direction * *distance}, .Distance = *distance};
    }
    return std::nullopt;
}

auto ray::intersect_rect(point_f topLeft, point_f topRight, point_f bottomLeft, point_f bottomRight) const -> std::vector<result>
{
    std::vector<result> retValue;
    if (auto distance {intersect_segment(_direction, point_d {topLeft}, point_d {topRight})}) { retValue.emplace_back(_origin + _direction * *distance, *distance); }
    if (auto distance {intersect_segment(_direction, point_d {topRight}, point_d {bottomRight})}) { retValue.emplace_back(_origin + _direction * *distance, *distance); }
    if (auto distance {intersect_segment(_direction, point_d {bottomRight}, point_d {bottomLeft})}) { retValue.emplace_back(_origin + _direction * *distance, *distance); }
    if (auto distance {intersect_segment(_direction, point_d {bottomLeft}, point_d {topLeft})}) { retValue.emplace_back(_origin + _direction * *distance, *distance); }
    return retValue;
}

auto ray::intersect_rect(rect_f const& rect, transform const& xform) const -> std::vector<result>
{
    point_f const topLeft {xform * rect.top_left()};
    point_f const topRight {xform * rect.top_right()};
    point_f const bottomLeft {xform * rect.bottom_left()};
    point_f const bottomRight {xform * rect.bottom_right()};
    return intersect_rect(topLeft, topRight, bottomLeft, bottomRight);
}

auto ray::intersect_circle(point_f const& center, f32 radius) const -> std::vector<result>
{
    f64 const b {2.0 * ((_origin.X - center.X) * _direction.X + (_origin.Y - center.Y) * _direction.Y)};
    f64 const c {((_origin.X - center.X) * (_origin.X - center.X)) + ((_origin.Y - center.Y) * (_origin.Y - center.Y)) - (radius * radius)};

    f64 const discr {(b * b) - (4 * c)};
    if (discr < 0) { return {}; }

    f64 const sqrtDiscr {std::sqrt(discr)};
    f64 const s1 {(-b - sqrtDiscr) / 2};
    f64 const s2 {(-b + sqrtDiscr) / 2};

    std::vector<result> retValue;
    if (s1 >= 0) { retValue.emplace_back(_origin + _direction * s1, s1); }
    if (s2 >= 0 && s2 != s1) { retValue.emplace_back(_origin + _direction * s2, s2); }
    return retValue;
}

auto ray::intersect_function(func const& func, f64 tolerance) const -> std::vector<result>
{
    std::vector<result> retValue;

    point_f lastPoint {func(0)};
    for (f64 t {0}; t <= 1; t += tolerance) {
        point_f const cp {func(t)};
        if (auto distance {intersect_segment(_direction, point_d {lastPoint}, point_d {cp})}) {
            auto const p {_origin + _direction * *distance};
            if (!retValue.empty() && retValue.back().Point == p) { continue; }
            retValue.emplace_back(p, *distance);
        }
        lastPoint = cp;
    }

    return retValue;
}

auto ray::get_point(f32 distance) -> point_f
{
    return _origin + (_direction * distance);
}

auto ray::intersect_segment(point_d const& rd, point_d const& p0, point_d const& p1) const -> std::optional<f64>
{
    point_d const ro {_origin};
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

}
