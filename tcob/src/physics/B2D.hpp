// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include "tcob/core/Point.hpp"
    #include "tcob/physics/Body.hpp"
    #include "tcob/physics/DebugDraw.hpp"
    #include "tcob/physics/Joint.hpp"
    #include "tcob/physics/Physics.hpp" // IWYU pragma: keep
    #include "tcob/physics/World.hpp"

    #include <box2d/box2d.h>

namespace tcob::physics::detail {

////////////////////////////////////////////////////////////

class b2d_world {
public:
    b2d_world(point_f gravity);
    ~b2d_world();

    void step(f32 delta, i32 subSteps) const;
    void set_gravity(point_f value) const;
    void set_enable_sleeping(bool value) const;
    void draw(b2d_debug_draw* draw, debug_draw::settings const& settings) const;
    auto get_contact_events(std::span<std::shared_ptr<body> const> bodies) const -> contact_events;

    b2WorldId ID {};
};

////////////////////////////////////////////////////////////

class b2d_body {
public:
    b2d_body(b2d_world* world, body_transform const& xform, body_settings const& bodySettings);
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

    b2BodyId ID {};
};

////////////////////////////////////////////////////////////

class b2d_joint {
public:
    b2d_joint(b2d_world* world, b2d_body const& bodyA, b2d_body const& bodyB, distance_joint_settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const& bodyA, b2d_body const& bodyB, motor_joint_settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const& bodyA, b2d_body const& bodyB, mouse_joint_settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const& bodyA, b2d_body const& bodyB, prismatic_joint_settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const& bodyA, b2d_body const& bodyB, revolute_joint_settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const& bodyA, b2d_body const& bodyB, weld_joint_settings const& jointSettings);
    b2d_joint(b2d_world* world, b2d_body const& bodyA, b2d_body const& bodyB, wheel_joint_settings const& jointSettings);
    ~b2d_joint();

    b2JointId ID {};
};

////////////////////////////////////////////////////////////

class b2d_shape {
public:
    b2d_shape(b2d_body* body, polygon_shape_settings const& shapeSettings);
    b2d_shape(b2d_body* body, rect_shape_settings const& shapeSettings);
    b2d_shape(b2d_body* body, circle_shape_settings const& shapeSettings);
    b2d_shape(b2d_body* body, segment_shape_settings const& shapeSettings);
    ~b2d_shape();

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
