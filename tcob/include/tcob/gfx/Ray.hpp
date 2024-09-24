// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"

namespace tcob::gfx {

class TCOB_API ray final {
public:
    ray(point_f origin, degree_d direction);

    point_f  Origin;
    degree_d Direction;

    auto intersect_line(point_f a, point_f b) const -> std::optional<point_f>;
    auto intersect_rect(rect_f const& rect) const -> std::vector<point_f>;
    auto intersect_circle(point_f const& center, f32 radius) -> std::vector<point_f>;

    auto intersect_polygon(auto&& polygon) const -> std::vector<point_f>;
    auto intersect_function(auto&& func, f64 tolerance = 0.01) const -> std::vector<point_f>;

private:
    auto intersect_segment(point_d const& rd, point_d const& p0, point_d const& p1) const -> std::optional<f64>;

    auto get_direction() const -> std::optional<point_d>;
};

}

#include "Ray.inl"
