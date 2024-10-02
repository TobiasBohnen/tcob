// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <initializer_list>
#include <span>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

using polyline      = std::vector<point_f>;
using polyline_span = std::span<point_f const>;

enum class clip_mode : u8 {
    Intersection,
    Union,
    Difference,
    Xor
};

////////////////////////////////////////////////////////////

class TCOB_API polygon final {
public:
    struct info {
        point_f Centroid;
        rect_f  BoundingBox;
    };

    polyline              Outline; // ccw
    std::vector<polyline> Holes;   // cw

    auto check_winding() const -> bool;
    auto get_info() const -> info;

    auto earcut() const -> std::vector<u32>;

    auto operator==(polygon const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API polygons final {
    using iterator       = std::vector<polygon>::iterator;
    using const_iterator = std::vector<polygon>::const_iterator;

public:
    polygons() = default;
    polygons(std::initializer_list<polygon> polygons);
    polygons(std::span<polygon const> polygons);

    auto begin() -> iterator;
    auto begin() const -> const_iterator;

    auto end() -> iterator;
    auto end() const -> const_iterator;

    auto add() -> polygon&;
    auto size() const -> isize;
    auto is_empty() const -> bool;
    void clear();

    auto check_winding() const -> bool;
    auto get_info() const -> polygon::info;

    void move_by(point_f offset);
    void clip(polygons const& other, clip_mode mode);

    auto operator==(polygons const& other) const -> bool = default;

private:
    std::vector<polygon> _polygons;
};

////////////////////////////////////////////////////////////

}
