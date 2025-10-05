// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <limits>
#include <optional>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Polygon.hpp"
#include "tcob/gfx/Transform.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API ray final {
public:
    ////////////////////////////////////////////////////////////
    struct result {
        point_f Point {};
        f64     Distance {0};

        auto operator==(result const&) const -> bool = default;
    };

    using func = std::function<point_f(f64)>;

    ////////////////////////////////////////////////////////////

    ray(point_f origin, degree_f direction, f64 maxDistance = std::numeric_limits<f64>::max());

    auto direction_vector() const -> point_d;

    auto get_point(f64 distance) const -> point_f;

    auto intersect_line(point_f a, point_f b) const -> std::optional<result>;

    auto intersect_rect(rect_f const& rect, transform const& xform = transform::Identity) const -> std::vector<result>;

    auto intersect_circle(point_f const& center, f32 radius) const -> std::vector<result>;

    auto intersect_function(func const& func, f64 tolerance = 0.01) const -> std::vector<result>;

    auto intersect_polyline(polyline_span polygon) const -> std::vector<result>;
    auto intersect_polyline(polyline_span polygon, transform const& xform) const -> std::vector<result>;

private:
    auto intersect_rect(point_f topLeft, point_f topRight, point_f bottomLeft, point_f bottomRight) const -> std::vector<result>;
    auto intersect_segment(point_d const& rd, point_d const& p0, point_d const& p1) const -> std::optional<f64>;

    auto get_result(f64 distance) const -> result;

    point_f _origin;
    f64     _maxDistance;
    point_d _direction;
};

}
