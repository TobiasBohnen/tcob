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

    #include "tcob/physics/Body.hpp"
    #include "tcob/physics/Physics.hpp"

namespace tcob::physics {
////////////////////////////////////////////////////////////

class TCOB_API debug_draw : public non_copyable {
    friend class world;

public:
    struct settings {
        /// Bounds to use if restricting drawing to a rectangular region
        std::optional<AABB> DrawingBounds {};

        /// Option to draw shapes
        bool DrawShapes {true};

        /// Option to draw joints
        bool DrawJoints {true};

        /// Option to draw additional information for joints
        bool DrawJointExtras {true};

        /// Option to draw the bounding boxes for shapes
        bool DrawAABBs {true};

        /// Option to draw the mass and center of mass of dynamic bodies
        bool DrawMass {true};

        /// Option to draw contact points
        bool DrawContacts {true};

        /// Option to visualize the graph coloring used for contacts and joints
        bool DrawGraphColors {true};

        /// Option to draw contact normals
        bool DrawContactNormals {true};

        /// Option to draw contact normal impulses
        bool DrawContactImpulses {true};

        /// Option to draw contact friction impulses
        bool DrawFrictionImpulses {true};
    };

    ////////////////////////////////////////////////////////////

    debug_draw();
    virtual ~debug_draw();

    settings Settings;

    /// Draw a closed polygon provided in CCW order.
    void virtual draw_polygon(std::span<point_f const> vertices, color color) = 0;

    /// Draw a solid closed polygon provided in CCW order.
    void virtual draw_solid_polygon(body_transform xform, std::span<point_f const> vertices, f32 radius, color color) = 0;

    /// Draw a circle.
    void virtual draw_circle(point_f center, f32 radius, color color) = 0;

    /// Draw a solid circle.
    void virtual draw_solid_circle(body_transform xform, f32 radius, color color) = 0;

    /// Draw a capsule.
    void virtual draw_capsule(point_f p1, point_f p2, f32 radius, color color) = 0;

    /// Draw a solid capsule.
    void virtual draw_solid_capsule(point_f p1, point_f p2, f32 radius, color color) = 0;

    /// Draw a line segment.
    void virtual draw_segment(point_f p1, point_f p2, color color) = 0;

    /// Draw a transform. Choose your own length scale.
    /// @param xf a transform.
    void virtual draw_transform(body_transform const& xf) = 0;

    /// Draw a point.
    void virtual draw_point(point_f p, f32 size, color color) = 0;

    /// Draw a string.
    void virtual draw_string(point_f p, string const& text) = 0;

private:
    std::unique_ptr<detail::b2d_debug_draw> _impl;
};

}

#endif
