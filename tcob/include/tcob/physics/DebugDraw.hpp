// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <optional>
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
        bool DrawBounds {true};

        /// Option to draw the mass and center of mass of dynamic bodies
        bool DrawMass {true};

        /// Option to draw body names
        bool DrawBodyNames {true};

        /// Option to draw contact points
        bool DrawContacts {true};

        /// Option to visualize the graph coloring used for contacts and joints
        bool DrawGraphColors {true};

        /// Option to draw contact normals
        bool DrawContactNormals {true};

        /// Option to draw contact normal impulses
        bool DrawContactImpulses {true};

        /// Option to draw contact feature ids
        bool DrawContactFeatures {true};

        /// Option to draw contact friction impulses
        bool DrawFrictionImpulses {true};

        /// Option to draw islands as bounding boxes
        bool DrawIslands {true};
    };

    ////////////////////////////////////////////////////////////

    debug_draw();
    virtual ~debug_draw();

    settings Settings;

    /// Draw a closed polygon provided in CCW order.
    virtual void draw_polygon(std::span<point_f const> vertices, color color) = 0;

    /// Draw a solid closed polygon provided in CCW order.
    virtual void draw_solid_polygon(body_transform xform, std::span<point_f const> vertices, f32 radius, color color) = 0;

    /// Draw a circle.
    virtual void draw_circle(point_f center, f32 radius, color color) = 0;

    /// Draw a solid circle.
    virtual void draw_solid_circle(body_transform xform, f32 radius, color color) = 0;

    /// Draw a solid capsule.
    virtual void draw_solid_capsule(point_f p1, point_f p2, f32 radius, color color) = 0;

    /// Draw a line segment.
    virtual void draw_segment(point_f p1, point_f p2, color color) = 0;

    /// Draw a transform. Choose your own length scale.
    /// @param xf a transform.
    virtual void draw_transform(body_transform const& xf) = 0;

    /// Draw a point.
    virtual void draw_point(point_f p, f32 size, color color) = 0;

    /// Draw a string.
    virtual void draw_string(point_f p, string const& text, color color) = 0;

private:
    std::unique_ptr<detail::b2d_debug_draw> _impl;
};

}
