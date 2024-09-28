// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Ray.hpp"

namespace tcob::gfx {

f32 constexpr epsilon {std::numeric_limits<f32>::epsilon()};

ray::ray(init init)
    : _init {init}
    , _direction {point_d::FromDirection(init.Direction)}
{
}

auto ray::get_direction_vector() const -> point_d
{
    return _direction;
}

auto ray::get_point(f64 distance) const -> point_f
{
    return _init.Origin + (_direction * distance);
}

auto ray::intersect_line(point_f a, point_f b) const -> std::optional<result>
{
    if (auto distance {intersect_segment(_direction, point_d {a}, point_d {b})}) {
        return result {.Point = get_point(*distance), .Distance = *distance};
    }
    return std::nullopt;
}

auto ray::intersect_rect(point_f topLeft, point_f topRight, point_f bottomLeft, point_f bottomRight) const -> std::vector<result>
{
    std::vector<result> retValue;
    if (auto const distance {intersect_segment(_direction, point_d {topLeft}, point_d {topRight})}) { retValue.emplace_back(get_result(*distance)); }
    if (auto const distance {intersect_segment(_direction, point_d {topRight}, point_d {bottomRight})}) { retValue.emplace_back(get_result(*distance)); }
    if (auto const distance {intersect_segment(_direction, point_d {bottomRight}, point_d {bottomLeft})}) { retValue.emplace_back(get_result(*distance)); }
    if (auto const distance {intersect_segment(_direction, point_d {bottomLeft}, point_d {topLeft})}) { retValue.emplace_back(get_result(*distance)); }
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
    f64 const b {2.0 * ((_init.Origin.X - center.X) * _direction.X + (_init.Origin.Y - center.Y) * _direction.Y)};
    f64 const c {((_init.Origin.X - center.X) * (_init.Origin.X - center.X)) + ((_init.Origin.Y - center.Y) * (_init.Origin.Y - center.Y)) - (radius * radius)};

    f64 const discr {(b * b) - (4 * c)};
    if (discr < 0) { return {}; }

    f64 const sqrtDiscr {std::sqrt(discr)};
    f64 const s1 {(-b - sqrtDiscr) / 2};
    f64 const s2 {(-b + sqrtDiscr) / 2};

    std::vector<result> retValue;
    if (s1 >= 0 && s1 <= _init.MaxDistance) { retValue.emplace_back(get_result(s1)); }
    if (s2 >= 0 && s2 != s1 && s2 <= _init.MaxDistance) { retValue.emplace_back(get_result(s2)); }
    return retValue;
}

auto ray::intersect_function(func const& func, f64 tolerance) const -> std::vector<result>
{
    std::vector<result> retValue;

    point_f lastPoint {func(0)};
    for (f64 t {0}; t <= 1; t += tolerance) {
        point_f const cp {func(t)};
        if (auto distance {intersect_segment(_direction, point_d {lastPoint}, point_d {cp})}) {
            auto const p {get_point(*distance)};
            if (!retValue.empty() && retValue.back().Point == p) { continue; }
            retValue.emplace_back(p, *distance);
        }
        lastPoint = cp;
    }

    return retValue;
}

auto ray::intersect_segment(point_d const& rd, point_d const& p0, point_d const& p1) const -> std::optional<f64>
{
    point_d const ro {_init.Origin};
    point_d const seg {p1 - p0};
    point_d const segPerp {seg.Y, -seg.X};

    f64 const denom {segPerp.dot(rd)};
    if (std::abs(denom) < epsilon) { return std::nullopt; }

    point_d const d {p0 - ro};
    f64 const     distance {segPerp.dot(d) / denom};
    if (distance < 0.0 || distance > _init.MaxDistance) { return std::nullopt; }

    f64 const s {point_d {rd.Y, -rd.X}.dot(d) / denom};
    if (s >= 0.0 && s <= 1.0) { return distance; }

    return std::nullopt;
}

auto ray::get_result(f64 distance) const -> result
{
    return {.Point = get_point(distance), .Distance = distance};
}

}
