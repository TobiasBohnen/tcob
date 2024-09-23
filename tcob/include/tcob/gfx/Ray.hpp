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
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {

class TCOB_API ray final {
public:
    ray(point_f origin, degree_d direction);

    point_f  Origin;
    degree_d Direction;

    auto intersect_line(point_f a, point_f b) const -> std::optional<point_f>;
    auto intersect_rect(rect_f const& rect) const -> std::vector<point_f>;
    auto intersect_polygon(polygon_span polygon) const -> std::vector<point_f>;

private:
    auto get_direction() const -> std::optional<point_d>;
};

}
