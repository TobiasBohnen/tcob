// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include "tcob/core/AngleUnits.hpp"
    #include "tcob/core/Color.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/physics/Body.hpp"
    #include "tcob/physics/Joint.hpp"
    #include "tcob/physics/Physics.hpp" // IWYU pragma: keep
    #include "tcob/physics/Shape.hpp"
    #include "tcob/physics/World.hpp"

    #include <box2d/box2d.h>
    #include <box2d/id.h>
    #include <box2d/types.h>

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

    void set_enable_sleeping(bool value) const;
    void set_enable_continuous(bool value) const;

    void set_restitution_threshold(f32 value) const;
    void set_hit_event_threshold(f32 value) const;

    void explode(point_f pos, f32 radius, f32 impulse) const;

    auto get_body_events() const -> body_events;
    auto get_contact_events() const -> contact_events;
    auto get_sensor_events() const -> sensor_events;

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

    void set_user_data(void* ptr) const;

    b2BodyId ID {};
};

////////////////////////////////////////////////////////////

class b2d_joint {
public:
    b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, distance_joint::settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, motor_joint::settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, mouse_joint::settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, prismatic_joint::settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, revolute_joint::settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, weld_joint::settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, wheel_joint::settings const& jointSettings);
    ~b2d_joint();

    void set_user_data(void* ptr) const;

    ////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////

    void distance_joint_set_length(f32 length) const;
    auto distance_joint_get_length() const -> f32;

    void distance_joint_enable_spring(bool enableSpring) const;
    auto distance_joint_is_spring_enabled() const -> bool;

    void distance_joint_set_spring_hertz(f32 hertz) const;
    auto distance_joint_get_hertz() const -> f32;

    void distance_joint_set_spring_damping_ratio(f32 dampingRatio) const;
    auto distance_joint_get_damping_ratio() const -> f32;

    void distance_joint_enable_limit(bool enableLimit) const;
    auto distance_joint_is_limit_enabled() const -> bool;

    void distance_joint_set_length_range(f32 minLength, f32 maxLength) const;

    auto distance_joint_get_min_length() const -> f32;
    auto distance_joint_get_max_length() const -> f32;

    auto distance_joint_get_current_length() const -> f32;

    void distance_joint_enable_motor(bool enableMotor) const;
    auto distance_joint_is_motor_enabled() const -> bool;

    void distance_joint_set_motor_speed(f32 motorSpeed) const;
    auto distance_joint_get_motor_speed() const -> f32;

    void distance_joint_set_max_motor_force(f32 force) const;
    auto distance_joint_get_max_motor_force() const -> f32;

    auto distance_joint_get_motor_force() const -> f32;

    ////////////////////////////////////////////////////////////

    void motor_joint_set_linear_offset(point_f linearOffset) const;
    auto motor_joint_get_linear_offset() const -> point_f;

    void motor_joint_set_angular_offset(f32 angularOffset) const;
    auto motor_joint_get_angular_offset() const -> f32;

    void motor_joint_set_max_force(f32 maxForce) const;
    auto motor_joint_get_max_force() const -> f32;

    void motor_joint_set_max_torque(f32 maxTorque) const;
    auto motor_joint_get_max_torque() const -> f32;

    void motor_joint_set_correction_factor(f32 correctionFactor) const;
    auto motor_joint_get_correction_factor() const -> f32;

    ////////////////////////////////////////////////////////////

    void mouse_joint_set_target(point_f target) const;
    auto mouse_joint_get_target() const -> point_f;

    void mouse_joint_set_spring_hertz(f32 hertz) const;
    auto mouse_joint_get_spring_hertz() const -> f32;

    void mouse_joint_set_spring_damping_ratio(f32 dampingRatio) const;
    auto mouse_joint_get_spring_damping_ratio() const -> f32;

    void mouse_joint_set_max_force(f32 maxForce) const;
    auto mouse_joint_get_max_force() const -> f32;

    ////////////////////////////////////////////////////////////

    void prismatic_joint_enable_spring(bool enableSpring) const;
    auto prismatic_joint_is_spring_enabled() const -> bool;

    void prismatic_joint_set_spring_hertz(f32 hertz) const;
    auto prismatic_joint_get_spring_hertz() const -> f32;

    void prismatic_joint_set_spring_damping_ratio(f32 dampingRatio) const;
    auto prismatic_joint_get_spring_damping_ratio() const -> f32;

    void prismatic_joint_enable_limit(bool enableLimit) const;
    auto prismatic_joint_is_limit_enabled() const -> bool;

    auto prismatic_joint_get_lower_limit() const -> f32;
    auto prismatic_joint_get_upper_limit() const -> f32;
    void prismatic_joint_set_limits(f32 lower, f32 upper) const;

    void prismatic_joint_enable_motor(bool enableMotor) const;
    auto prismatic_joint_is_motor_enabled() const -> bool;

    void prismatic_joint_set_motor_speed(f32 motorSpeed) const;
    auto prismatic_joint_get_motor_speed() const -> f32;

    void prismatic_joint_set_max_motor_force(f32 force) const;
    auto prismatic_joint_get_max_motor_force() const -> f32;

    auto prismatic_joint_get_motor_force() const -> f32;

    ////////////////////////////////////////////////////////////

    void revolute_joint_enable_spring(bool enableSpring) const;
    auto revolute_joint_is_spring_enabled() const -> bool;

    void revolute_joint_set_spring_hertz(f32 hertz) const;
    auto revolute_joint_get_spring_hertz() const -> f32;

    void revolute_joint_set_spring_damping_ratio(f32 dampingRatio) const;
    auto revolute_joint_get_spring_damping_ratio() const -> f32;

    auto revolute_joint_get_angle() const -> radian_f;

    void revolute_joint_enable_limit(bool enableLimit) const;
    auto revolute_joint_is_limit_enabled() const -> bool;

    auto revolute_joint_get_lower_limit() const -> f32;
    auto revolute_joint_get_upper_limit() const -> f32;
    void revolute_joint_set_limits(f32 lower, f32 upper) const;

    void revolute_joint_enable_motor(bool enableMotor) const;
    auto revolute_joint_is_motor_enabled() const -> bool;

    void revolute_joint_set_motor_speed(f32 motorSpeed) const;
    auto revolute_joint_get_motor_speed() const -> f32;

    auto revolute_joint_get_motor_torque() const -> f32;

    void revolute_joint_set_max_motor_torque(f32 torque) const;
    auto revolute_joint_get_max_motor_torque() const -> f32;

    ////////////////////////////////////////////////////////////

    void weld_joint_set_linear_hertz(f32 hertz) const;
    auto weld_joint_get_linear_hertz() const -> f32;

    void weld_joint_set_linear_damping_ratio(f32 dampingRatio) const;
    auto weld_joint_get_linear_damping_ratio() const -> f32;

    void weld_joint_set_angular_hertz(f32 hertz) const;
    auto weld_joint_get_angular_hertz() const -> f32;

    void weld_joint_set_angular_damping_ratio(f32 dampingRatio) const;
    auto weld_joint_get_angular_damping_ratio() const -> f32;

    ////////////////////////////////////////////////////////////

    void wheel_joint_enable_spring(bool enableSpring) const;
    auto wheel_joint_is_spring_enabled() const -> bool;

    void wheel_joint_set_spring_hertz(f32 hertz) const;
    auto wheel_joint_get_spring_hertz() const -> f32;

    void wheel_joint_set_spring_damping_ratio(f32 dampingRatio) const;
    auto wheel_joint_get_spring_damping_ratio() const -> f32;

    void wheel_joint_enable_limit(bool enableLimit) const;
    auto wheel_joint_is_limit_enabled() const -> bool;

    auto wheel_joint_get_lower_limit() const -> f32;
    auto wheel_joint_get_upper_limit() const -> f32;
    void wheel_joint_set_limits(f32 lower, f32 upper) const;

    void wheel_joint_enable_motor(bool enableMotor) const;
    auto wheel_joint_is_motor_enabled() const -> bool;

    void wheel_joint_set_motor_speed(f32 motorSpeed) const;
    auto wheel_joint_get_motor_speed() const -> f32;

    void wheel_joint_set_max_motor_torque(f32 torque) const;
    auto wheel_joint_get_max_motor_torque() const -> f32;

    auto wheel_joint_get_motor_torque() const -> f32;

    ////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////

    b2JointId ID {};
};

////////////////////////////////////////////////////////////

class b2d_shape {
public:
    b2d_shape(b2d_body* body, polygon_shape::settings const& shapeSettings);
    b2d_shape(b2d_body* body, rect_shape::settings const& shapeSettings);
    b2d_shape(b2d_body* body, circle_shape::settings const& shapeSettings);
    b2d_shape(b2d_body* body, segment_shape::settings const& shapeSettings);
    b2d_shape(b2d_body* body, capsule_shape::settings const& shapeSettings);
    ~b2d_shape();

    auto is_sensor() const -> bool;

    void set_density(f32 density) const;
    auto get_density() const -> f32;

    void set_friction(f32 friction) const;
    auto get_friction() const -> f32;

    void set_restitution(f32 restitution) const;
    auto get_restitution() const -> f32;

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

    b2ShapeId ID {};
};

////////////////////////////////////////////////////////////

class b2d_debug_draw {
public:
    b2d_debug_draw(debug_draw* parent);

    void draw_polygon(std::span<point_f const> vertices, color color);
    void draw_solid_polygon(body_transform xform, std::span<point_f const> vertices, f32 radius, color color);
    void draw_circle(point_f center, f32 radius, color color);
    void draw_solid_circle(body_transform xform, f32 radius, color color);
    void draw_capsule(point_f p1, point_f p2, f32 radius, color color);
    void draw_solid_capsule(point_f p1, point_f p2, f32 radius, color color);
    void draw_segment(point_f p1, point_f p2, color color);
    void draw_transform(body_transform const& xf);
    void draw_point(point_f p, f32 size, color color);
    void draw_string(point_f p, string const& text);

    void apply_settings(debug_draw::settings const& settings);

    b2DebugDraw ID {};

private:
    debug_draw* _parent;
};

}

#endif
