// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

enum class clip_mode : u8 {
    Intersection,
    Union,
    Difference,
    Xor
};

enum class offset_join : u8 {
    Square,
    Bevel,
    Round,
    Miter
};

////////////////////////////////////////////////////////////

using polyline      = std::vector<point_f>;
using polyline_span = std::span<point_f const>;

////////////////////////////////////////////////////////////

class TCOB_API polygon final {
public:
    struct info {
        point_f Centroid;
        rect_f  BoundingBox;
    };

    polyline              Outline; // ccw
    std::vector<polyline> Holes;   // cw

    auto get_info() const -> info; // TODO: get_

    auto earcut [[nodiscard]] () const -> std::vector<u32>;

    void apply_transform(transform const& xform);

    auto operator==(polygon const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

namespace polygons {
    TCOB_API auto get_winding(polyline_span polyline) -> winding;
    TCOB_API auto check_winding(std::span<polygon const> polygons) -> bool;

    TCOB_API auto is_point_inside(point_f point, polyline_span polyline) -> bool;

    TCOB_API auto get_info(std::span<polygon const> polygons) -> polygon::info;
    TCOB_API auto get_info(polyline_span polyline) -> polygon::info;

    TCOB_API void move_by(std::span<polygon> polygons, point_f offset);

    TCOB_API void clip(std::vector<polygon>& polygons, std::span<polygon const> other, clip_mode mode);
    TCOB_API void offset(std::vector<polygon>& polygons, f64 delta, offset_join join);
};

////////////////////////////////////////////////////////////

}
