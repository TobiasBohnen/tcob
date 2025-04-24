// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <vector>

    #include <box2d/box2d.h>
    #include <box2d/id.h>
    #include <box2d/types.h>

    #include "tcob/core/AngleUnits.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/core/Rect.hpp"
    #include "tcob/physics/Body.hpp"
    #include "tcob/physics/Joint.hpp"
    #include "tcob/physics/Physics.hpp" // IWYU pragma: keep
    #include "tcob/physics/Shape.hpp"
    #include "tcob/physics/World.hpp"

namespace tcob::physics::detail {
////////////////////////////////////////////////////////////

class b2d_world {
public:
    b2d_world(world::settings const& settings);
    ~b2d_world();

    void step(f32 delta, i32 subSteps) const;

    void draw(b2d_debug_draw* draw, debug_draw::settings const& settings) const;

    auto get_gravity() const -> point_f;
    void set_gravity(point_f value) const;

    auto get_enable_sleeping() const -> bool;
    void set_enable_sleeping(bool value) const;

    auto get_enable_continuous() const -> bool;
    void set_enable_continuous(bool value) const;

    auto get_restitution_threshold() const -> f32;
    void set_restitution_threshold(f32 value) const;

    auto get_hit_event_threshold() const -> f32;
    void set_hit_event_threshold(f32 value) const;

    auto get_maximum_linear_speed() const -> f32;
    void set_maximum_linear_speed(f32 value) const;

    void explode(explosion const& explosion) const;

    auto get_body_events() const -> body_events;
    auto get_contact_events() const -> contact_events;
    auto get_sensor_events() const -> sensor_events;

    void set_joint_tuning(f32 hertz, f32 damping) const;
    void set_contact_tuning(f32 hertz, f32 damping, f32 pushSpeed) const;

    auto get_awake_body_count() const -> i32;

    auto get_enable_warm_starting() const -> bool;
    void set_enable_warm_starting(bool value) const;

    b2WorldId ID {};
};

////////////////////////////////////////////////////////////

class b2d_body {
public:
    b2d_body(b2d_world* world, body_transform const& xform, body::settings const& bodySettings);
    ~b2d_body();

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

    auto get_enable_sleep() const -> bool;
    void set_enable_sleep(bool value) const;

    auto get_sleep_threshold() const -> f32;
    void set_sleep_threshold(f32 value) const;

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

    auto get_name() const -> string;
    void set_name(string const& value) const;

    auto get_mass() const -> f32;

    auto get_mass_data() const -> mass_data;
    void set_mass_data(mass_data const& value) const;

    void apply_force(point_f force, point_f point, bool wake) const;
    void apply_force_to_center(point_f force, bool wake) const;
    void apply_linear_impulse(point_f imp, point_f point, bool wake) const;
    void apply_linear_impulse_to_center(point_f imp, bool wake) const;
    void apply_torque(f32 torque, bool wake) const;
    void apply_angular_impulse(f32 impulse, bool wake) const;

    void set_user_data(void* ptr) const;

    void enable_contact_events(bool value) const;
    void enable_hit_events(bool value) const;

    auto compute_aabb() const -> rect_f;

    auto get_position() const -> point_f;
    auto get_rotation() const -> radian_f;

    auto get_local_point(point_f pos) const -> point_f;
    auto get_world_point(point_f pos) const -> point_f;

    auto get_local_vector(point_f pos) const -> point_f;
    auto get_world_vector(point_f pos) const -> point_f;

    auto get_local_point_velocity(point_f pos) const -> point_f;
    auto get_world_point_velocity(point_f pos) const -> point_f;

    void set_target_transform(body_transform xform, f32 timeStep) const;

    auto get_rotational_inertia() const -> f32;

    void apply_mass_from_shapes() const;

    b2BodyId ID {};
};

////////////////////////////////////////////////////////////

class b2d_joint {
public:
    b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, distance_joint::settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, motor_joint::settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, mouse_joint::settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, filter_joint::settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, prismatic_joint::settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, revolute_joint::settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, weld_joint::settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, wheel_joint::settings const& jointSettings);
    ~b2d_joint();

    void set_user_data(void* ptr) const;

    auto get_body_a() const -> body*;
    auto get_body_b() const -> body*;
    void wake_bodies() const;

    auto get_local_anchor_a() const -> point_f;
    auto get_local_anchor_b() const -> point_f;

    auto get_constraint_force() const -> point_f;
    auto get_constraint_torque() const -> f32;

    auto get_collide_connected() const -> bool;
    void set_collide_connected(bool value) const;

    ////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////

    auto distance_joint_get_length() const -> f32;
    void distance_joint_set_length(f32 length) const;

    auto distance_joint_is_spring_enabled() const -> bool;
    void distance_joint_enable_spring(bool enableSpring) const;

    auto distance_joint_get_spring_hertz() const -> f32;
    void distance_joint_set_spring_hertz(f32 hertz) const;

    auto distance_joint_get_spring_damping_ratio() const -> f32;
    void distance_joint_set_spring_damping_ratio(f32 dampingRatio) const;

    auto distance_joint_is_limit_enabled() const -> bool;
    void distance_joint_enable_limit(bool enableLimit) const;

    void distance_joint_set_length_range(f32 minLength, f32 maxLength) const;

    auto distance_joint_get_min_length() const -> f32;
    auto distance_joint_get_max_length() const -> f32;

    auto distance_joint_get_current_length() const -> f32;

    auto distance_joint_is_motor_enabled() const -> bool;
    void distance_joint_enable_motor(bool enableMotor) const;

    auto distance_joint_get_motor_speed() const -> f32;
    void distance_joint_set_motor_speed(f32 motorSpeed) const;

    auto distance_joint_get_max_motor_force() const -> f32;
    void distance_joint_set_max_motor_force(f32 force) const;

    auto distance_joint_get_motor_force() const -> f32;

    ////////////////////////////////////////////////////////////

    auto motor_joint_get_linear_offset() const -> point_f;
    void motor_joint_set_linear_offset(point_f linearOffset) const;

    auto motor_joint_get_angular_offset() const -> f32;
    void motor_joint_set_angular_offset(f32 angularOffset) const;

    auto motor_joint_get_max_force() const -> f32;
    void motor_joint_set_max_force(f32 maxForce) const;

    auto motor_joint_get_max_torque() const -> f32;
    void motor_joint_set_max_torque(f32 maxTorque) const;

    auto motor_joint_get_correction_factor() const -> f32;
    void motor_joint_set_correction_factor(f32 correctionFactor) const;

    ////////////////////////////////////////////////////////////

    auto mouse_joint_get_target() const -> point_f;
    void mouse_joint_set_target(point_f target) const;

    auto mouse_joint_get_spring_hertz() const -> f32;
    void mouse_joint_set_spring_hertz(f32 hertz) const;

    auto mouse_joint_get_spring_damping_ratio() const -> f32;
    void mouse_joint_set_spring_damping_ratio(f32 dampingRatio) const;

    auto mouse_joint_get_max_force() const -> f32;
    void mouse_joint_set_max_force(f32 maxForce) const;

    ////////////////////////////////////////////////////////////

    auto prismatic_joint_is_spring_enabled() const -> bool;
    void prismatic_joint_enable_spring(bool enableSpring) const;

    auto prismatic_joint_get_spring_hertz() const -> f32;
    void prismatic_joint_set_spring_hertz(f32 hertz) const;

    auto prismatic_joint_get_spring_damping_ratio() const -> f32;
    void prismatic_joint_set_spring_damping_ratio(f32 dampingRatio) const;

    auto prismatic_joint_is_limit_enabled() const -> bool;
    void prismatic_joint_enable_limit(bool enableLimit) const;

    auto prismatic_joint_get_lower_limit() const -> f32;
    auto prismatic_joint_get_upper_limit() const -> f32;
    void prismatic_joint_set_limits(f32 lower, f32 upper) const;

    auto prismatic_joint_is_motor_enabled() const -> bool;
    void prismatic_joint_enable_motor(bool enableMotor) const;

    auto prismatic_joint_get_motor_speed() const -> f32;
    void prismatic_joint_set_motor_speed(f32 motorSpeed) const;

    auto prismatic_joint_get_max_motor_force() const -> f32;
    void prismatic_joint_set_max_motor_force(f32 force) const;

    auto prismatic_joint_get_motor_force() const -> f32;

    auto prismatic_joint_get_translation() const -> f32;
    auto prismatic_joint_get_speed() const -> f32;

    ////////////////////////////////////////////////////////////

    auto revolute_joint_is_spring_enabled() const -> bool;
    void revolute_joint_enable_spring(bool enableSpring) const;

    auto revolute_joint_get_spring_hertz() const -> f32;
    void revolute_joint_set_spring_hertz(f32 hertz) const;

    auto revolute_joint_get_spring_damping_ratio() const -> f32;
    void revolute_joint_set_spring_damping_ratio(f32 dampingRatio) const;

    auto revolute_joint_get_angle() const -> radian_f;

    auto revolute_joint_is_limit_enabled() const -> bool;
    void revolute_joint_enable_limit(bool enableLimit) const;

    auto revolute_joint_get_lower_limit() const -> f32;
    auto revolute_joint_get_upper_limit() const -> f32;
    void revolute_joint_set_limits(f32 lower, f32 upper) const;

    auto revolute_joint_is_motor_enabled() const -> bool;
    void revolute_joint_enable_motor(bool enableMotor) const;

    auto revolute_joint_get_motor_speed() const -> f32;
    void revolute_joint_set_motor_speed(f32 motorSpeed) const;

    auto revolute_joint_get_motor_torque() const -> f32;

    void revolute_joint_set_max_motor_torque(f32 torque) const;
    auto revolute_joint_get_max_motor_torque() const -> f32;

    ////////////////////////////////////////////////////////////

    auto weld_joint_get_linear_hertz() const -> f32;
    void weld_joint_set_linear_hertz(f32 hertz) const;

    auto weld_joint_get_linear_damping_ratio() const -> f32;
    void weld_joint_set_linear_damping_ratio(f32 dampingRatio) const;

    auto weld_joint_get_angular_hertz() const -> f32;
    void weld_joint_set_angular_hertz(f32 hertz) const;

    auto weld_joint_get_angular_damping_ratio() const -> f32;
    void weld_joint_set_angular_damping_ratio(f32 dampingRatio) const;

    ////////////////////////////////////////////////////////////

    auto wheel_joint_is_spring_enabled() const -> bool;
    void wheel_joint_enable_spring(bool enableSpring) const;

    auto wheel_joint_get_spring_hertz() const -> f32;
    void wheel_joint_set_spring_hertz(f32 hertz) const;

    auto wheel_joint_get_spring_damping_ratio() const -> f32;
    void wheel_joint_set_spring_damping_ratio(f32 dampingRatio) const;

    auto wheel_joint_is_limit_enabled() const -> bool;
    void wheel_joint_enable_limit(bool enableLimit) const;

    auto wheel_joint_get_lower_limit() const -> f32;
    auto wheel_joint_get_upper_limit() const -> f32;
    void wheel_joint_set_limits(f32 lower, f32 upper) const;

    auto wheel_joint_is_motor_enabled() const -> bool;
    void wheel_joint_enable_motor(bool enableMotor) const;

    auto wheel_joint_get_motor_speed() const -> f32;
    void wheel_joint_set_motor_speed(f32 motorSpeed) const;

    auto wheel_joint_get_max_motor_torque() const -> f32;
    void wheel_joint_set_max_motor_torque(f32 torque) const;

    auto wheel_joint_get_motor_torque() const -> f32;

    ////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////

    b2JointId ID {};
};

////////////////////////////////////////////////////////////

class b2d_shape {
public:
    b2d_shape(b2d_body* body, polygon_shape::settings const& settings, shape::settings const& shapeSettings);
    b2d_shape(b2d_body* body, rect_shape::settings const& settings, shape::settings const& shapeSettings);
    b2d_shape(b2d_body* body, circle_shape::settings const& settings, shape::settings const& shapeSettings);
    b2d_shape(b2d_body* body, segment_shape::settings const& settings, shape::settings const& shapeSettings);
    b2d_shape(b2d_body* body, capsule_shape::settings const& settings, shape::settings const& shapeSettings);
    ~b2d_shape();

    auto is_sensor() const -> bool;

    auto get_density() const -> f32;
    void set_density(f32 density) const;

    auto get_friction() const -> f32;
    void set_friction(f32 friction) const;

    auto get_restitution() const -> f32;
    void set_restitution(f32 restitution) const;

    void enable_sensor_events(bool flag) const;
    auto are_sensor_events_enabled() const -> bool;

    void enable_contact_events(bool flag) const;
    auto are_contact_events_enabled() const -> bool;

    void enable_pre_solve_events(bool flag) const;
    auto are_pre_solve_events_enabled() const -> bool;

    void enable_hit_events(bool flag) const;
    auto are_hit_events_enabled() const -> bool;

    auto test_point(point_f point) const -> bool;

    auto get_aabb() const -> AABB;

    auto get_closest_point(point_f target) const -> point_f;

    void set_user_data(void* ptr) const;

    auto equal(b2d_shape const* other) const -> bool;

    auto get_mass_data() const -> mass_data;

    auto get_filter() const -> filter;
    void set_filter(filter const& value) const;

    auto get_sensor_overlaps() const -> std::vector<shape*>;

    b2ShapeId ID {};
};

////////////////////////////////////////////////////////////

class b2d_chain {
public:
    b2d_chain(b2d_body* body, chain::settings const& shapeSettings);
    ~b2d_chain();

    auto get_friction() const -> f32;
    void set_friction(f32 value) const;

    auto get_restitution() const -> f32;
    void set_restitution(f32 value) const;

    auto get_segments() const -> std::vector<chain_segment>;

    b2ChainId ID {};
};

////////////////////////////////////////////////////////////

class b2d_debug_draw {
public:
    b2d_debug_draw(debug_draw* impl);

    void apply_settings(debug_draw::settings const& settings);

    b2DebugDraw ID {};

    debug_draw* Impl;
};

////////////////////////////////////////////////////////////

auto rot_from_angle(radian_f angle) -> rotation;

}

#endif

/*
MISSING API:
+b2Body_GetContactData
+b2Shape_GetContactData

-b2Body_GetShapeCount
-b2Body_GetShapes
-b2Body_GetJointCount
-b2Body_GetJoints
-b2Body_GetContactCapacity

-b2Chain_SetMaterial
-b2Chain_GetMaterial

-b2Joint_GetType( b2JointId jointId );
-b2Joint_GetWorld( b2JointId jointId );

-b2Shape_GetContactCapacity
-b2Shape_GetSensorCapacity
-b2Shape_SetMaterial
-b2Shape_GetMaterial
-b2Shape_RayCast

+b2Shape_GetCircle( b2ShapeId shapeId );
+b2Shape_GetSegment( b2ShapeId shapeId );
+b2Shape_GetChainSegment( b2ShapeId shapeId );
+b2Shape_GetCapsule( b2ShapeId shapeId );
+b2Shape_GetPolygon( b2ShapeId shapeId );

b2World_CastShape
b2World_CastMover
b2World_GetCounters
b2World_SetFrictionCallback
b2World_SetRestitutionCallback
b2World_SetCustomFilterCallback
b2World_SetPreSolveCallback
b2World_OverlapAABB
b2World_OverlapShape
b2World_CastRay
b2World_CastRayClosest
b2World_CollideMover
*/
