// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <span>

    #include "tcob/core/Color.hpp"
    #include "tcob/core/Interfaces.hpp"
    #include "tcob/core/Point.hpp"

    #include "tcob/physics/box2d/B2D.hpp"
    #include "tcob/physics/box2d/B2DBody.hpp"

namespace tcob::physics::box2d {
////////////////////////////////////////////////////////////

struct ddraw;

class TCOB_API debug_draw : public non_copyable {
    friend auto detail::get_impl(debug_draw const* b) -> b2Draw*;

public:
    debug_draw();
    virtual ~debug_draw();

    /// Draw a closed polygon provided in CCW order.
    void virtual draw_polygon(std::span<point_f const> vertices, color color) = 0;

    /// Draw a solid closed polygon provided in CCW order.
    void virtual draw_solid_polygon(std::span<point_f const> vertices, color color) = 0;

    /// Draw a circle.
    void virtual draw_circle(point_f center, f32 radius, color color) = 0;

    /// Draw a solid circle.
    void virtual draw_solid_circle(point_f center, f32 radius, point_f axis, color color) = 0;

    /// Draw a line segment.
    void virtual draw_segment(point_f p1, point_f p2, color color) = 0;

    /// Draw a transform. Choose your own length scale.
    /// @param xf a transform.
    void virtual draw_transform(body_transform const& xf) = 0;

    /// Draw a point.
    void virtual draw_point(point_f p, f32 size, color color) = 0;

private:
    b2Draw* _b2Draw {nullptr};
};

}

#endif
