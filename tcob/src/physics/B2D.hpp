// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include "tcob/core/Point.hpp"
    #include "tcob/physics/Body.hpp"
    #include "tcob/physics/Physics.hpp" // IWYU pragma: keep
    #include "tcob/physics/World.hpp"   // IWYU pragma: keep

    #include <box2d/box2d.h>

namespace tcob::physics::detail {

////////////////////////////////////////////////////////////

class b2dWorld {
public:
    b2dWorld(point_f gravity);
    ~b2dWorld();

    b2WorldId ID {};

    void step(f32 delta, i32 subSteps) const;
    void set_gravity(point_f value) const;
    void set_allow_sleeping(bool value) const;
};

////////////////////////////////////////////////////////////

class b2dBody {
public:
    b2dBody(b2dWorld* world, body_transform const& xform, body_settings const& bodySettings);
    ~b2dBody();

    auto get_type() const -> body_type;
    void set_type(body_type type) const;

    auto get_linear_velocity() const -> point_f;
    void set_linear_velocity(point_f value) const;

    auto get_angular_velocity() const -> radian_f;
    void set_angular_velocity(radian_f value) const;

    auto get_linear_damping() const -> f32;
    void set_linear_damping(f32 value) const;

    auto get_angular_damping() const -> f32;
    void set_angular_damping(f32 value) const;

    auto get_allow_sleep() const -> bool;
    void set_allow_sleep(bool value) const;

    auto get_awake() const -> bool;
    void set_awake(bool value) const;

    auto get_fixed_rotation() const -> bool;
    void set_fixed_rotation(bool value) const;

    auto get_bullet() const -> bool;
    void set_bullet(bool value) const;

    auto get_enabled() const -> bool;
    void set_enabled(bool value) const;

    auto get_gravity_scale() const -> f32;
    void set_gravity_scale(f32 value) const;

    auto get_transform() const -> body_transform;
    void set_transform(body_transform value) const;

    auto get_center() const -> point_f;
    auto get_local_center() const -> point_f;

    void apply_force(point_f force, point_f point, bool wake) const;
    void apply_force_to_center(point_f force, bool wake) const;
    void apply_linear_impulse(point_f imp, point_f point, bool wake) const;
    void apply_linear_impulse_to_center(point_f imp, bool wake) const;
    void apply_torque(f32 torque, bool wake) const;
    void apply_angular_impulse(f32 impulse, bool wake) const;

    b2BodyId ID {};
};

////////////////////////////////////////////////////////////

class b2dJoint {
public:
    b2dJoint(b2dWorld* world, distance_joint_settings const& jointSettings);
    b2dJoint(b2dWorld* world, motor_joint_settings const& jointSettings);
    b2dJoint(b2dWorld* world, mouse_joint_settings const& jointSettings);
    b2dJoint(b2dWorld* world, prismatic_joint_settings const& jointSettings);
    b2dJoint(b2dWorld* world, revolute_joint_settings const& jointSettings);
    b2dJoint(b2dWorld* world, weld_joint_settings const& jointSettings);
    b2dJoint(b2dWorld* world, wheel_joint_settings const& jointSettings);
    ~b2dJoint();

    b2JointId ID {};
};

////////////////////////////////////////////////////////////

class b2dShape {
public:
    b2dShape(b2dBody* body, polygon_shape_settings const& shapeSettings);
    b2dShape(b2dBody* body, rect_shape_settings const& shapeSettings);
    b2dShape(b2dBody* body, circle_shape_settings const& shapeSettings);
    b2dShape(b2dBody* body, segment_shape_settings const& shapeSettings);
    ~b2dShape();

    b2ShapeId ID {};
};

}

#endif
