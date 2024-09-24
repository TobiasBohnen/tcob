// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Ray.hpp"

namespace tcob::gfx {

inline auto ray::intersect_polygon(auto&& polygon) const -> std::vector<point_f>
{
    auto const r {get_direction()};
    if (!r) { return {}; }

    std::vector<point_f> retValue;

    usize const n {polygon.size()};
    for (usize i {0}; i < n; ++i) {
        if (auto distance {intersect_segment(*r, point_d {polygon[i]}, point_d {polygon[(i + 1) % n]})}) {
            retValue.emplace_back(Origin + *r * *distance);
        }
    }

    return retValue;
}

inline auto ray::intersect_function(auto&& func, f64 tolerance) const -> std::vector<point_f>
{
    auto const r {get_direction()};
    if (!r) { return {}; }

    std::vector<point_f> retValue;

    point_f lastPoint {func(0)};
    for (f64 t {0}; t <= 1; t += tolerance) {
        point_f const cp {func(t)};
        if (auto distance {intersect_segment(*r, point_d {lastPoint}, point_d {cp})}) {
            auto const p {Origin + *r * *distance};
            if (!retValue.empty() && retValue.back() == p) { continue; }
            retValue.push_back(p);
        }
        lastPoint = cp;
    }

    return retValue;
}

}
