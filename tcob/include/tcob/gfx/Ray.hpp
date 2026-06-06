// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <limits>
#include <optional>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Polygon.hpp"

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

    using func               = std::function<point_f(f64)>;
    using intersect_callback = std::function<void(result const&)>;

    ////////////////////////////////////////////////////////////

    ray(point_f origin, degree_f direction, f64 maxDistance = std::numeric_limits<f64>::max());

    auto get_point(f64 distance) const -> point_f;

    void intersect_line(point_f a, point_f b, intersect_callback const& fn) const;

    void intersect_rect(rect_f const& rect, intersect_callback const& fn) const;
    void intersect_rect(point_f topLeft, point_f topRight, point_f bottomLeft, point_f bottomRight, intersect_callback const& fn) const;

    void intersect_circle(point_f const& center, f64 radius, intersect_callback const& fn) const;

    void intersect_function(func const& func, f64 tolerance, intersect_callback const& fn) const;

    void intersect_polyline(polyline_span polygon, intersect_callback const& fn) const;

private:
    auto intersect_segment(point_d const& rd, point_d const& p0, point_d const& p1) const -> std::optional<f64>;

    auto get_result(f64 distance) const -> result;

    point_f _origin;
    f64     _maxDistance;
    point_d _direction;
};

}
