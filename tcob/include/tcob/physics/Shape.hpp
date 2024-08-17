// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/core/Property.hpp"
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <span>

    #include "tcob/core/AngleUnits.hpp"
    #include "tcob/core/Interfaces.hpp"
    #include "tcob/core/Rect.hpp"
    #include "tcob/physics/Physics.hpp"

namespace tcob::physics {
////////////////////////////////////////////////////////////

class TCOB_API shape : public non_copyable {
    friend auto detail::get_impl(shape const& t) -> detail::b2d_shape*;

public:
    class TCOB_API settings {
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
        bool EnableHitEvents {true};

        /// Enable pre-solve contact events for this shape. Only applies to dynamic bodies. These are expensive
        ///	and must be carefully handled due to threading. Ignored for sensors.
        bool EnablePreSolveEvents {false};
    };

    prop_fn<f32>  Friction;
    prop_fn<f32>  Restitution;
    prop_fn<f32>  Density;
    prop_fn<bool> EnableSensorEvents;
    prop_fn<bool> EnableContactEvents;
    prop_fn<bool> EnableHitEvents;
    prop_fn<bool> EnablePreSolveEvents;

    auto get_body() -> body&;

    auto is_sensor() const -> bool;

    auto get_aabb() const -> AABB;

    auto test_point(point_f point) const -> bool;

    auto get_closest_point(point_f target) const -> point_f;

    auto operator==(shape const& other) const -> bool;

protected:
    shape(body& body, std::unique_ptr<detail::b2d_shape> impl);
    ~shape();

private:
    std::unique_ptr<detail::b2d_shape> _impl;
    body&                              _body;
};

////////////////////////////////////////////////////////////

class TCOB_API polygon_shape final : public shape {
    friend class body;

public:
    class TCOB_API settings : public shape::settings {
    public:
        std::span<point_f const> Verts;
        f32                      Radius {};
    };

private:
    polygon_shape(body& body, settings const& shapeSettings);
};

////////////////////////////////////////////////////////////

class TCOB_API rect_shape final : public shape {
    friend class body;

public:
    class TCOB_API settings : public shape::settings {
    public:
        rect_f   Extents {};
        radian_f Angle {};
    };

private:
    rect_shape(body& body, settings const& shapeSettings);
};

////////////////////////////////////////////////////////////

class TCOB_API circle_shape final : public shape {
    friend class body;

public:
    class TCOB_API settings : public shape::settings {
    public:
        point_f Center {};
        f32     Radius {};
    };

private:
    circle_shape(body& body, settings const& shapeSettings);
};

////////////////////////////////////////////////////////////

class TCOB_API segment_shape final : public shape {
    friend class body;

public:
    class TCOB_API settings : public shape::settings {
    public:
        point_f Point0 {};
        point_f Point1 {};
    };

private:
    segment_shape(body& body, settings const& shapeSettings);
};

////////////////////////////////////////////////////////////

class TCOB_API capsule_shape final : public shape {
    friend class body;

public:
    class TCOB_API settings : public shape::settings {
    public:
        point_f Center0 {};
        point_f Center1 {};
        f32     Radius {0};
    };

private:
    capsule_shape(body& body, settings const& shapeSettings);
};
}

#endif
