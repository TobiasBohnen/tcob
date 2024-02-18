// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <span>

    #include "tcob/core/AngleUnits.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/core/Property.hpp"
    #include "tcob/physics/box2d/B2D.hpp"
    #include "tcob/physics/box2d/B2DFixture.hpp"

namespace tcob::physics::box2d {
////////////////////////////////////////////////////////////

struct body_transform {
    /// The world position of the body. Avoid creating bodies at the origin
    /// since this can lead to many overlapping shapes.
    point_f Position {point_f::Zero};

    /// The world angle of the body in radians.
    radian_f Angle {0.0f};
};

inline auto operator==(body_transform const& left, body_transform const& right) -> bool
{
    return (left.Position == right.Position)
        && (left.Angle == right.Angle);
}

////////////////////////////////////////////////////////////

struct body_settings {
    /// The body type: static, kinematic, or dynamic.
    /// Note: if a dynamic body would have zero mass, the mass is set to one.
    body_type Type {body_type::Static};

    /// The linear velocity of the body's origin in world co-ordinates.
    point_f LinearVelocity {point_f::Zero};

    /// The angular velocity of the body.
    radian_f AngularVelocity {0.0f};

    /// Linear damping is use to reduce the linear velocity. The damping parameter
    /// can be larger than 1.0f but the damping effect becomes sensitive to the
    /// time step when the damping parameter is large.
    /// Units are 1/time
    f32 LinearDamping {0.0f};

    /// Angular damping is use to reduce the angular velocity. The damping parameter
    /// can be larger than 1.0f but the damping effect becomes sensitive to the
    /// time step when the damping parameter is large.
    /// Units are 1/time
    f32 AngularDamping {0.0f};

    /// Set this flag to false if this body should never fall asleep. Note that
    /// this increases CPU usage.
    bool AllowSleep {true};

    /// Is this body initially awake or sleeping?
    bool IsAwake {true};

    /// Should this body be prevented from rotating? Useful for characters.
    bool IsFixedRotation {false};

    /// Is this a fast moving body that should be prevented from tunneling through
    /// other moving bodies? Note that all bodies are prevented from tunneling through
    /// kinematic and static bodies. This setting is only considered on dynamic bodies.
    /// @warning You should use this flag sparingly since it increases processing time.
    bool IsBullet {false};

    /// Does this body start out enabled?
    bool IsEnabled {true};

    /// Scale the gravity applied to this body.
    f32 GravityScale {1.0f};
};

class TCOB_API body final {
    friend auto detail::get_impl(body const* b) -> b2Body*;
    friend class world;

public:
    prop_fn<body_type>      Type;
    prop_fn<point_f>        LinearVelocity;
    prop_fn<radian_f>       AngularVelocity;
    prop_fn<f32>            LinearDamping;
    prop_fn<f32>            AngularDamping;
    prop_fn<bool>           AllowSleep;
    prop_fn<bool>           Awake;
    prop_fn<bool>           IsFixedRotation;
    prop_fn<bool>           IsBullet;
    prop_fn<bool>           Enabled;
    prop_fn<f32>            GravityScale;
    prop_fn<body_transform> Transform;
    prop<std::any>          UserData;

    auto get_center() const -> point_f;
    auto get_local_center() const -> point_f;
    auto get_world() const -> world&;
    auto get_fixtures() -> std::span<std::shared_ptr<fixture>>;

    auto create_fixture(shape const& shape, fixture_settings const& settings) -> std::shared_ptr<fixture>;
    void destroy_fixture(std::shared_ptr<fixture> const& fixturePtr);

    void wake_up() const;
    void sleep() const;

    void apply_force(point_f force, point_f point, bool wake = true) const;
    void apply_force_to_center(point_f force, bool wake = true) const;
    void apply_linear_impulse(point_f imp, point_f point, bool wake = true) const;
    void apply_linear_impulse_to_center(point_f imp, bool wake = true) const;
    void apply_torgue(f32 torgue, bool wake = true) const;
    void apply_angular_impulse(f32 impulse, bool wake = true) const;

private:
    body(b2Body* body, world* parent);

    std::vector<std::shared_ptr<fixture>> _fixtures;
    b2Body*                               _b2Body;
    world*                                _world;
};

////////////////////////////////////////////////////////////

inline auto operator==(body const& left, body const& right) -> bool
{
    return detail::get_impl(&left) == detail::get_impl(&right);
}

}

#endif
