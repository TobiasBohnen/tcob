// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <span>

    #include "tcob/core/AngleUnits.hpp"
    #include "tcob/core/Interfaces.hpp"
    #include "tcob/core/Rect.hpp"
    #include "tcob/physics/Physics.hpp"

namespace tcob::physics {
////////////////////////////////////////////////////////////

class TCOB_API shape_settings {
public:
    /// The Coulomb (dry) friction coefficient, usually in the range [0,1].
    f32 Friction {0.6f};

    /// The restitution (bounce) usually in the range [0,1].
    f32 Restitution {0.0f};

    /// The density, usually in kg/m^2.
    f32 Density {1.0f};

    /// A sensor shape generates overlap events but never generates a collision response.
    bool IsSensor {false};

    /// Enable sensor events for this shape. Only applies to kinematic and dynamic bodies. Ignored for sensors.
    bool EnableSensorEvents {true};

    /// Enable contact events for this shape. Only applies to kinematic and dynamic bodies. Ignored for sensors.
    bool EnableContactEvents {true};

    /// Enable hit events for this shape. Only applies to kinematic and dynamic bodies. Ignored for sensors.
    bool EnableHitEvents {false};

    /// Enable pre-solve contact events for this shape. Only applies to dynamic bodies. These are expensive
    ///	and must be carefully handled due to threading. Ignored for sensors.
    bool EnablePreSolveEvents {false};
};

class TCOB_API shape : public non_copyable {
protected:
    explicit shape(std::unique_ptr<detail::b2d_shape> impl);
    ~shape();

private:
    std::unique_ptr<detail::b2d_shape> _impl;
};

////////////////////////////////////////////////////////////

class TCOB_API polygon_shape_settings : public shape_settings {
public:
    std::span<point_f const> Verts;
    f32                      Radius {};
};

class TCOB_API polygon_shape final : public shape {
    friend class body;

private:
    polygon_shape(detail::b2d_body* body, polygon_shape_settings const& shapeSettings);
};

////////////////////////////////////////////////////////////

class TCOB_API rect_shape_settings : public shape_settings {
public:
    rect_f   Extents {};
    radian_f Angle {};
};

class TCOB_API rect_shape final : public shape {
    friend class body;

private:
    rect_shape(detail::b2d_body* body, rect_shape_settings const& shapeSettings);
};

////////////////////////////////////////////////////////////

class TCOB_API circle_shape_settings : public shape_settings {
public:
    point_f Center {};
    f32     Radius {};
};

class TCOB_API circle_shape final : public shape {
    friend class body;

private:
    circle_shape(detail::b2d_body* body, circle_shape_settings const& shapeSettings);
};

////////////////////////////////////////////////////////////

class TCOB_API segment_shape_settings : public shape_settings {
public:
    point_f Point0 {};
    point_f Point1 {};
};

class TCOB_API segment_shape final : public shape {
    friend class body;

private:
    segment_shape(detail::b2d_body* body, segment_shape_settings const& shapeSettings);
};

}

#endif
