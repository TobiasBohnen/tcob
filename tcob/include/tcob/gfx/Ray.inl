// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Ray.hpp"

namespace tcob::gfx {

inline auto ray::intersect_polygon(auto&& polygon) const -> std::vector<result>
{
    std::vector<result> retValue;

    usize const n {polygon.size()};
    for (usize i {0}; i < n; ++i) {
        if (auto distance {intersect_segment(_direction, point_d {polygon[i]}, point_d {polygon[(i + 1) % n]})}) {
            retValue.emplace_back(get_result(*distance));
        }
    }

    return retValue;
}

inline auto ray::intersect_polygon(auto&& polygon, transform const& xform) const -> std::vector<result>
{
    std::vector<point_f> points;
    points.reserve(polygon.size());
    for (auto const& point : polygon) {
        points.push_back(xform * point);
    }
    return intersect_polygon(points);
}

}
