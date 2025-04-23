// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <any>
    #include <memory>
    #include <span>
    #include <vector>

    #include "tcob/core/AngleUnits.hpp"
    #include "tcob/core/Interfaces.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/core/Property.hpp"
    #include "tcob/core/Rect.hpp"
    #include "tcob/physics/Physics.hpp"

namespace tcob::physics {
////////////////////////////////////////////////////////////

class TCOB_API shape : public non_copyable {
public:
    class TCOB_API settings {
    public:
        /// The surface material for this shape.
        surface_material Material;

        /// The density, usually in kg/m^2.
        f32 Density {1.0f};

        /// Collision filtering data.
        filter Filter;

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

        /// When shapes are created they will scan the environment for collision the next time step. This can significantly slow down
        /// static body creation when there are many static shapes.
        /// This is flag is ignored for dynamic and kinematic shapes which always invoke contact creation.
        bool InvokeContactCreation {true};

        /// Should the body update the mass properties when this shape is created. Default is true.
        bool UpdateBodyMass {true};
    };

    prop_fn<f32>  Friction;
    prop_fn<f32>  Restitution;
    prop_fn<f32>  Density;
    prop_fn<bool> EnableSensorEvents;
    prop_fn<bool> EnableContactEvents;
    prop_fn<bool> EnableHitEvents;
    prop_fn<bool> EnablePreSolveEvents;

    std::any UserData;

    auto parent() -> body&;

    auto is_sensor() const -> bool;
    auto sensor_overlaps() const -> std::vector<shape*>;

    auto aabb() const -> AABB;
    auto mass_data() const -> mass_data;

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
    polygon_shape(body& body, detail::b2d_body* b2dBody, settings const& shapeSettings);
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
    rect_shape(body& body, detail::b2d_body* b2dBody, settings const& shapeSettings);
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
    circle_shape(body& body, detail::b2d_body* b2dBody, settings const& shapeSettings);
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
    segment_shape(body& body, detail::b2d_body* b2dBody, settings const& shapeSettings);
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
    capsule_shape(body& body, detail::b2d_body* b2dBody, settings const& shapeSettings);
};
}

#endif
