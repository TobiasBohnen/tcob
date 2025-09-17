// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/Joint.hpp"

#include <cassert>
#include <memory>
#include <utility>

#include "B2D.hpp"

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/physics/Body.hpp"
#include "tcob/physics/Physics.hpp"
#include "tcob/physics/World.hpp"

namespace tcob::physics {

joint::joint(world& world, std::unique_ptr<detail::b2d_joint> impl)
    : IsCollideConnected {detail::make_prop<bool, &detail::b2d_joint::get_collide_connected, &detail::b2d_joint::set_collide_connected>(this)}
    , _impl {std::move(impl)}
    , _world {world}
{
    _impl->set_user_data(this);
}

joint::~joint() = default;

auto joint::operator==(joint const& other) const -> bool
{
    return _impl.get() == other._impl.get();
}

auto joint::parent() -> world&
{
    return _world;
}

auto joint::body_a() const -> body*
{
    return _impl->get_body_a();
}

auto joint::body_b() const -> body*
{
    return _impl->get_body_b();
}

void joint::wake_bodies() const
{
    _impl->wake_bodies();
}

auto joint::local_anchor_a() const -> point_f
{
    return _impl->get_local_anchor_a();
}

auto joint::local_anchor_b() const -> point_f
{
    return _impl->get_local_anchor_b();
}

auto joint::constraint_force() const -> point_f
{
    return _impl->get_constraint_force();
}

auto joint::constraint_torque() const -> f32
{
    return _impl->get_constraint_torque();
}

void joint::set_constraint_tuning(f32 hertz, f32 dampingRatio) const
{
    _impl->set_constraint_tuning(hertz, dampingRatio);
}

auto joint::get_body_impl(body* body) const -> detail::b2d_body*
{
    assert(body);
    return body->_impl.get();
}

auto joint::get_impl() const -> detail::b2d_joint*
{
    return _impl.get();
}

////////////////////////////////////////////////////////////

distance_joint::distance_joint(world& world, detail::b2d_world* b2dWorld, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(b2dWorld, get_body_impl(jointSettings.BodyA), get_body_impl(jointSettings.BodyB), jointSettings)}
    , Length {detail::make_prop<f32, &detail::b2d_joint::distance_joint_get_length, &detail::b2d_joint::distance_joint_set_length>(this)}
    , EnableSpring {detail::make_prop<bool, &detail::b2d_joint::distance_joint_is_spring_enabled, &detail::b2d_joint::distance_joint_enable_spring>(this)}
    , Hertz {detail::make_prop<f32, &detail::b2d_joint::distance_joint_get_spring_hertz, &detail::b2d_joint::distance_joint_set_spring_hertz>(this)}
    , DampingRatio {detail::make_prop<f32, &detail::b2d_joint::distance_joint_get_spring_damping_ratio, &detail::b2d_joint::distance_joint_set_spring_damping_ratio>(this)}
    , EnableLimit {detail::make_prop<bool, &detail::b2d_joint::distance_joint_is_limit_enabled, &detail::b2d_joint::distance_joint_enable_limit>(this)}
    , MinLength {detail::make_prop<f32, &detail::b2d_joint::distance_joint_get_min_length, &detail::b2d_joint::distance_joint_set_min_length>(this)}
    , MaxLength {detail::make_prop<f32, &detail::b2d_joint::distance_joint_get_max_length, &detail::b2d_joint::distance_joint_set_max_length>(this)}
    , EnableMotor {detail::make_prop<bool, &detail::b2d_joint::distance_joint_is_motor_enabled, &detail::b2d_joint::distance_joint_enable_motor>(this)}
    , MotorSpeed {detail::make_prop<f32, &detail::b2d_joint::distance_joint_get_motor_speed, &detail::b2d_joint::distance_joint_set_motor_speed>(this)}
    , MaxMotorForce {detail::make_prop<f32, &detail::b2d_joint::distance_joint_get_max_motor_force, &detail::b2d_joint::distance_joint_set_max_motor_force>(this)}
{
}

auto distance_joint::current_length() const -> f32
{
    return get_impl()->distance_joint_get_current_length();
}

auto distance_joint::motor_force() const -> f32
{
    return get_impl()->distance_joint_get_motor_force();
}

////////////////////////////////////////////////////////////

motor_joint::motor_joint(world& world, detail::b2d_world* b2dWorld, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(b2dWorld, get_body_impl(jointSettings.BodyA), get_body_impl(jointSettings.BodyB), jointSettings)}
    , LinearOffset {detail::make_prop<point_f, &detail::b2d_joint::motor_joint_get_linear_offset, &detail::b2d_joint::motor_joint_set_linear_offset>(this)}
    , AngularOffset {detail::make_prop<f32, &detail::b2d_joint::motor_joint_get_angular_offset, &detail::b2d_joint::motor_joint_set_angular_offset>(this)}
    , MaxForce {detail::make_prop<f32, &detail::b2d_joint::motor_joint_get_max_force, &detail::b2d_joint::motor_joint_set_max_force>(this)}
    , MaxTorque {detail::make_prop<f32, &detail::b2d_joint::motor_joint_get_max_torque, &detail::b2d_joint::motor_joint_set_max_torque>(this)}
    , CorrectionFactor {detail::make_prop<f32, &detail::b2d_joint::motor_joint_get_correction_factor, &detail::b2d_joint::motor_joint_set_correction_factor>(this)}
{
}

////////////////////////////////////////////////////////////

mouse_joint::mouse_joint(world& world, detail::b2d_world* b2dWorld, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(b2dWorld, get_body_impl(jointSettings.BodyA), get_body_impl(jointSettings.BodyB), jointSettings)}
    , Target {detail::make_prop<point_f, &detail::b2d_joint::mouse_joint_get_target, &detail::b2d_joint::mouse_joint_set_target>(this)}
    , Hertz {detail::make_prop<f32, &detail::b2d_joint::mouse_joint_get_spring_hertz, &detail::b2d_joint::mouse_joint_set_spring_hertz>(this)}
    , DampingRatio {detail::make_prop<f32, &detail::b2d_joint::mouse_joint_get_spring_damping_ratio, &detail::b2d_joint::mouse_joint_set_spring_damping_ratio>(this)}
    , MaxForce {detail::make_prop<f32, &detail::b2d_joint::mouse_joint_get_max_force, &detail::b2d_joint::mouse_joint_set_max_force>(this)}
{
}

////////////////////////////////////////////////////////////

filter_joint::filter_joint(world& world, detail::b2d_world* b2dWorld, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(b2dWorld, get_body_impl(jointSettings.BodyA), get_body_impl(jointSettings.BodyB), jointSettings)}
{
}

////////////////////////////////////////////////////////////

prismatic_joint::prismatic_joint(world& world, detail::b2d_world* b2dWorld, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(b2dWorld, get_body_impl(jointSettings.BodyA), get_body_impl(jointSettings.BodyB), jointSettings)}
    , EnableSpring {detail::make_prop<bool, &detail::b2d_joint::prismatic_joint_is_spring_enabled, &detail::b2d_joint::prismatic_joint_enable_spring>(this)}
    , Hertz {detail::make_prop<f32, &detail::b2d_joint::prismatic_joint_get_spring_hertz, &detail::b2d_joint::prismatic_joint_set_spring_hertz>(this)}
    , DampingRatio {detail::make_prop<f32, &detail::b2d_joint::prismatic_joint_get_spring_damping_ratio, &detail::b2d_joint::prismatic_joint_set_spring_damping_ratio>(this)}
    , EnableLimit {detail::make_prop<bool, &detail::b2d_joint::prismatic_joint_is_limit_enabled, &detail::b2d_joint::prismatic_joint_enable_limit>(this)}
    , LowerTranslation {detail::make_prop<f32, &detail::b2d_joint::prismatic_joint_get_lower_limit, &detail::b2d_joint::prismatic_joint_set_lower_limit>(this)}
    , UpperTranslation {detail::make_prop<f32, &detail::b2d_joint::prismatic_joint_get_upper_limit, &detail::b2d_joint::prismatic_joint_set_upper_limit>(this)}
    , EnableMotor {detail::make_prop<bool, &detail::b2d_joint::prismatic_joint_is_motor_enabled, &detail::b2d_joint::prismatic_joint_enable_motor>(this)}
    , MaxMotorForce {detail::make_prop<f32, &detail::b2d_joint::prismatic_joint_get_max_motor_force, &detail::b2d_joint::prismatic_joint_set_max_motor_force>(this)}
    , MotorSpeed {detail::make_prop<f32, &detail::b2d_joint::prismatic_joint_get_motor_speed, &detail::b2d_joint::prismatic_joint_set_motor_speed>(this)}
{
}

auto prismatic_joint::motor_force() const -> f32
{
    return get_impl()->prismatic_joint_get_motor_force();
}

auto prismatic_joint::translation() const -> f32
{
    return get_impl()->prismatic_joint_get_translation();
}

auto prismatic_joint::speed() const -> f32
{
    return get_impl()->prismatic_joint_get_speed();
}

////////////////////////////////////////////////////////////

revolute_joint::revolute_joint(world& world, detail::b2d_world* b2dWorld, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(b2dWorld, get_body_impl(jointSettings.BodyA), get_body_impl(jointSettings.BodyB), jointSettings)}
    , EnableSpring {detail::make_prop<bool, &detail::b2d_joint::revolute_joint_is_spring_enabled, &detail::b2d_joint::revolute_joint_enable_spring>(this)}
    , Hertz {detail::make_prop<f32, &detail::b2d_joint::revolute_joint_get_spring_hertz, &detail::b2d_joint::revolute_joint_set_spring_hertz>(this)}
    , DampingRatio {detail::make_prop<f32, &detail::b2d_joint::revolute_joint_get_spring_damping_ratio, &detail::b2d_joint::revolute_joint_set_spring_damping_ratio>(this)}
    , EnableLimit {detail::make_prop<bool, &detail::b2d_joint::revolute_joint_is_limit_enabled, &detail::b2d_joint::revolute_joint_enable_limit>(this)}
    , LowerAngle {detail::make_prop<f32, &detail::b2d_joint::revolute_joint_get_lower_limit, &detail::b2d_joint::revolute_joint_set_lower_limit>(this)}
    , UpperAngle {detail::make_prop<f32, &detail::b2d_joint::revolute_joint_get_upper_limit, &detail::b2d_joint::revolute_joint_set_upper_limit>(this)}
    , EnableMotor {detail::make_prop<bool, &detail::b2d_joint::revolute_joint_is_motor_enabled, &detail::b2d_joint::revolute_joint_enable_motor>(this)}
    , MaxMotorTorque {detail::make_prop<f32, &detail::b2d_joint::revolute_joint_get_max_motor_torque, &detail::b2d_joint::revolute_joint_set_max_motor_torque>(this)}
    , MotorSpeed {detail::make_prop<f32, &detail::b2d_joint::revolute_joint_get_motor_speed, &detail::b2d_joint::revolute_joint_set_motor_speed>(this)}
{
}

auto revolute_joint::angle() const -> radian_f
{
    return get_impl()->revolute_joint_get_angle();
}

auto revolute_joint::motor_torque() const -> f32
{
    return get_impl()->revolute_joint_get_motor_torque();
}

////////////////////////////////////////////////////////////

weld_joint::weld_joint(world& world, detail::b2d_world* b2dWorld, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(b2dWorld, get_body_impl(jointSettings.BodyA), get_body_impl(jointSettings.BodyB), jointSettings)}
    , LinearHertz {detail::make_prop<f32, &detail::b2d_joint::weld_joint_get_linear_hertz, &detail::b2d_joint::weld_joint_set_linear_hertz>(this)}
    , AngularHertz {detail::make_prop<f32, &detail::b2d_joint::weld_joint_get_angular_hertz, &detail::b2d_joint::weld_joint_set_angular_hertz>(this)}
    , LinearDampingRatio {detail::make_prop<f32, &detail::b2d_joint::weld_joint_get_linear_damping_ratio, &detail::b2d_joint::weld_joint_set_linear_damping_ratio>(this)}
    , AngularDampingRatio {detail::make_prop<f32, &detail::b2d_joint::weld_joint_get_angular_damping_ratio, &detail::b2d_joint::weld_joint_set_angular_damping_ratio>(this)}
{
}

////////////////////////////////////////////////////////////

wheel_joint::wheel_joint(world& world, detail::b2d_world* b2dWorld, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(b2dWorld, get_body_impl(jointSettings.BodyA), get_body_impl(jointSettings.BodyB), jointSettings)}
    , EnableSpring {detail::make_prop<bool, &detail::b2d_joint::wheel_joint_is_spring_enabled, &detail::b2d_joint::wheel_joint_enable_spring>(this)}
    , Hertz {detail::make_prop<f32, &detail::b2d_joint::wheel_joint_get_spring_hertz, &detail::b2d_joint::wheel_joint_set_spring_hertz>(this)}
    , DampingRatio {detail::make_prop<f32, &detail::b2d_joint::wheel_joint_get_spring_damping_ratio, &detail::b2d_joint::wheel_joint_set_spring_damping_ratio>(this)}
    , EnableLimit {detail::make_prop<bool, &detail::b2d_joint::wheel_joint_is_limit_enabled, &detail::b2d_joint::wheel_joint_enable_limit>(this)}
    , LowerTranslation {detail::make_prop<f32, &detail::b2d_joint::wheel_joint_get_lower_limit, &detail::b2d_joint::wheel_joint_set_lower_limit>(this)}
    , UpperTranslation {detail::make_prop<f32, &detail::b2d_joint::wheel_joint_get_upper_limit, &detail::b2d_joint::wheel_joint_set_upper_limit>(this)}
    , EnableMotor {detail::make_prop<bool, &detail::b2d_joint::wheel_joint_is_motor_enabled, &detail::b2d_joint::wheel_joint_enable_motor>(this)}
    , MaxMotorTorque {detail::make_prop<f32, &detail::b2d_joint::wheel_joint_get_max_motor_torque, &detail::b2d_joint::wheel_joint_set_max_motor_torque>(this)}
    , MotorSpeed {detail::make_prop<f32, &detail::b2d_joint::wheel_joint_get_motor_speed, &detail::b2d_joint::wheel_joint_set_motor_speed>(this)}
{
}

auto wheel_joint::motor_torque() const -> f32
{
    return get_impl()->wheel_joint_get_motor_torque();
}

}
