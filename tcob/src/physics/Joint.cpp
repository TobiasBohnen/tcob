// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/Joint.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <cassert>
    #include <memory>
    #include <utility>

    #include "B2D.hpp"

    #include "tcob/core/AngleUnits.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/physics/Body.hpp"
    #include "tcob/physics/World.hpp"

namespace tcob::physics {

joint::joint(world& world, std::unique_ptr<detail::b2d_joint> impl)
    : _impl {std::move(impl)}
    , _world {world}
{
    _impl->set_user_data(this);
}

joint::~joint() = default;

auto joint::operator==(joint const& other) const -> bool
{
    return _impl.get() == other._impl.get();
}

auto joint::get_world() -> world&
{
    return _world;
}

auto joint::get_body_impl(body* body) const -> detail::b2d_body*
{
    assert(body);
    return body->_impl.get();
}

auto joint::get_impl() const -> detail::b2d_joint&
{
    return *_impl;
}

////////////////////////////////////////////////////////////

distance_joint::distance_joint(world& world, detail::b2d_world* b2dWorld, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(b2dWorld, get_body_impl(jointSettings.BodyA), get_body_impl(jointSettings.BodyB), jointSettings)}
    , Length {{[this]() -> f32 { return get_impl().distance_joint_get_length(); },
               [this](auto const& value) { get_impl().distance_joint_set_length(value); }}}
    , EnableSpring {{[this]() -> bool { return get_impl().distance_joint_is_spring_enabled(); },
                     [this](auto const& value) { get_impl().distance_joint_enable_spring(value); }}}
    , Hertz {{[this]() -> f32 { return get_impl().distance_joint_get_spring_hertz(); },
              [this](auto const& value) { get_impl().distance_joint_set_spring_hertz(value); }}}
    , DampingRatio {{[this]() -> f32 { return get_impl().distance_joint_get_spring_damping_ratio(); },
                     [this](auto const& value) { get_impl().distance_joint_set_spring_damping_ratio(value); }}}
    , EnableLimit {{[this]() -> bool { return get_impl().distance_joint_is_limit_enabled(); },
                    [this](auto const& value) { get_impl().distance_joint_enable_limit(value); }}}
    , MinLength {{[this]() -> f32 { return get_impl().distance_joint_get_min_length(); },
                  [this](auto const& value) { get_impl().distance_joint_set_length_range(value, get_impl().distance_joint_get_max_length()); }}}
    , MaxLength {{[this]() -> f32 { return get_impl().distance_joint_get_max_length(); },
                  [this](auto const& value) { get_impl().distance_joint_set_length_range(get_impl().distance_joint_get_min_length(), value); }}}
    , EnableMotor {{[this]() -> bool { return get_impl().distance_joint_is_motor_enabled(); },
                    [this](auto const& value) { get_impl().distance_joint_enable_motor(value); }}}
    , MotorSpeed {{[this]() -> f32 { return get_impl().distance_joint_get_motor_speed(); },
                   [this](auto const& value) { get_impl().distance_joint_set_motor_speed(value); }}}
    , MaxMotorForce {{[this]() -> f32 { return get_impl().distance_joint_get_max_motor_force(); },
                      [this](auto const& value) { get_impl().distance_joint_set_motor_speed(value); }}}
{
}

auto distance_joint::current_length() const -> f32
{
    return get_impl().distance_joint_get_current_length();
}

auto distance_joint::motor_force() const -> f32
{
    return get_impl().distance_joint_get_motor_force();
}

////////////////////////////////////////////////////////////

motor_joint::motor_joint(world& world, detail::b2d_world* b2dWorld, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(b2dWorld, get_body_impl(jointSettings.BodyA), get_body_impl(jointSettings.BodyB), jointSettings)}
    , LinearOffset {{[this]() -> point_f { return get_impl().motor_joint_get_linear_offset(); },
                     [this](auto const& value) { get_impl().motor_joint_set_linear_offset(value); }}}
    , AngularOffset {{[this]() -> f32 { return get_impl().motor_joint_get_angular_offset(); },
                      [this](auto const& value) { get_impl().motor_joint_set_angular_offset(value); }}}
    , MaxForce {{[this]() -> f32 { return get_impl().motor_joint_get_max_force(); },
                 [this](auto const& value) { get_impl().motor_joint_set_max_force(value); }}}
    , MaxTorque {{[this]() -> f32 { return get_impl().motor_joint_get_max_torque(); },
                  [this](auto const& value) { get_impl().motor_joint_set_max_torque(value); }}}
    , CorrectionFactor {{[this]() -> f32 { return get_impl().motor_joint_get_correction_factor(); },
                         [this](auto const& value) { get_impl().motor_joint_set_correction_factor(value); }}}
{
}

////////////////////////////////////////////////////////////

mouse_joint::mouse_joint(world& world, detail::b2d_world* b2dWorld, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(b2dWorld, get_body_impl(jointSettings.BodyA), get_body_impl(jointSettings.BodyB), jointSettings)}
    , Target {{[this]() -> point_f { return get_impl().mouse_joint_get_target(); },
               [this](auto const& value) { get_impl().mouse_joint_set_target(value); }}}
    , Hertz {{[this]() -> f32 { return get_impl().mouse_joint_get_spring_hertz(); },
              [this](auto const& value) { get_impl().mouse_joint_set_spring_hertz(value); }}}
    , DampingRatio {{[this]() -> f32 { return get_impl().mouse_joint_get_spring_damping_ratio(); },
                     [this](auto const& value) { get_impl().mouse_joint_set_spring_damping_ratio(value); }}}
    , MaxForce {{[this]() -> f32 { return get_impl().mouse_joint_get_max_force(); },
                 [this](auto const& value) { get_impl().mouse_joint_set_max_force(value); }}}
{
}

////////////////////////////////////////////////////////////

prismatic_joint::prismatic_joint(world& world, detail::b2d_world* b2dWorld, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(b2dWorld, get_body_impl(jointSettings.BodyA), get_body_impl(jointSettings.BodyB), jointSettings)}
    , EnableSpring {{[this]() -> bool { return get_impl().prismatic_joint_is_spring_enabled(); },
                     [this](auto const& value) { get_impl().prismatic_joint_enable_spring(value); }}}
    , Hertz {{[this]() -> f32 { return get_impl().prismatic_joint_get_spring_hertz(); },
              [this](auto const& value) { get_impl().prismatic_joint_set_spring_hertz(value); }}}
    , DampingRatio {{[this]() -> f32 { return get_impl().prismatic_joint_get_spring_damping_ratio(); },
                     [this](auto const& value) { get_impl().prismatic_joint_set_spring_damping_ratio(value); }}}
    , EnableLimit {{[this]() -> bool { return get_impl().prismatic_joint_is_limit_enabled(); },
                    [this](auto const& value) { get_impl().prismatic_joint_enable_limit(value); }}}
    , LowerTranslation {{[this]() -> f32 { return get_impl().prismatic_joint_get_lower_limit(); },
                         [this](auto const& value) { get_impl().prismatic_joint_set_limits(value, get_impl().prismatic_joint_get_upper_limit()); }}}
    , UpperTranslation {{[this]() -> f32 { return get_impl().prismatic_joint_get_upper_limit(); },
                         [this](auto const& value) { get_impl().prismatic_joint_set_limits(get_impl().prismatic_joint_get_lower_limit(), value); }}}
    , EnableMotor {{[this]() -> bool { return get_impl().prismatic_joint_is_motor_enabled(); },
                    [this](auto const& value) { get_impl().prismatic_joint_enable_motor(value); }}}
    , MaxMotorForce {{[this]() -> f32 { return get_impl().prismatic_joint_get_max_motor_force(); },
                      [this](auto const& value) { get_impl().prismatic_joint_set_max_motor_force(value); }}}
    , MotorSpeed {{[this]() -> f32 { return get_impl().prismatic_joint_get_motor_speed(); },
                   [this](auto const& value) { get_impl().prismatic_joint_set_motor_speed(value); }}}
{
}

auto prismatic_joint::motor_force() const -> f32
{
    return get_impl().prismatic_joint_get_motor_force();
}

auto prismatic_joint::translation() const -> f32
{
    return get_impl().prismatic_joint_get_translation();
}

auto prismatic_joint::speed() const -> f32
{
    return get_impl().prismatic_joint_get_speed();
}

////////////////////////////////////////////////////////////

revolute_joint::revolute_joint(world& world, detail::b2d_world* b2dWorld, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(b2dWorld, get_body_impl(jointSettings.BodyA), get_body_impl(jointSettings.BodyB), jointSettings)}
    , EnableSpring {{[this]() -> bool { return get_impl().revolute_joint_is_spring_enabled(); },
                     [this](auto const& value) { get_impl().revolute_joint_enable_spring(value); }}}
    , Hertz {{[this]() -> f32 { return get_impl().revolute_joint_get_spring_hertz(); },
              [this](auto const& value) { get_impl().revolute_joint_set_spring_hertz(value); }}}
    , DampingRatio {{[this]() -> f32 { return get_impl().revolute_joint_get_spring_damping_ratio(); },
                     [this](auto const& value) { get_impl().revolute_joint_set_spring_damping_ratio(value); }}}
    , EnableLimit {{[this]() -> bool { return get_impl().revolute_joint_is_limit_enabled(); },
                    [this](auto const& value) { get_impl().revolute_joint_enable_limit(value); }}}
    , LowerAngle {{[this]() -> f32 { return get_impl().revolute_joint_get_lower_limit(); },
                   [this](auto const& value) { get_impl().revolute_joint_set_limits(value, get_impl().revolute_joint_get_upper_limit()); }}}
    , UpperAngle {{[this]() -> f32 { return get_impl().revolute_joint_get_upper_limit(); },
                   [this](auto const& value) { get_impl().revolute_joint_set_limits(get_impl().revolute_joint_get_lower_limit(), value); }}}
    , EnableMotor {{[this]() -> bool { return get_impl().revolute_joint_is_motor_enabled(); },
                    [this](auto const& value) { get_impl().revolute_joint_enable_motor(value); }}}
    , MaxMotorTorque {{[this]() -> f32 { return get_impl().revolute_joint_get_max_motor_torque(); },
                       [this](auto const& value) { get_impl().revolute_joint_set_max_motor_torque(value); }}}
    , MotorSpeed {{[this]() -> f32 { return get_impl().revolute_joint_get_motor_speed(); },
                   [this](auto const& value) { get_impl().revolute_joint_set_motor_speed(value); }}}
{
}

auto revolute_joint::angle() const -> radian_f
{
    return get_impl().revolute_joint_get_angle();
}

auto revolute_joint::motor_torque() const -> f32
{
    return get_impl().revolute_joint_get_motor_torque();
}

////////////////////////////////////////////////////////////

weld_joint::weld_joint(world& world, detail::b2d_world* b2dWorld, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(b2dWorld, get_body_impl(jointSettings.BodyA), get_body_impl(jointSettings.BodyB), jointSettings)}
    , LinearHertz {{[this]() -> f32 { return get_impl().weld_joint_get_linear_hertz(); },
                    [this](auto const& value) { get_impl().weld_joint_set_linear_hertz(value); }}}
    , AngularHertz {{[this]() -> f32 { return get_impl().weld_joint_get_angular_hertz(); },
                     [this](auto const& value) { get_impl().weld_joint_set_angular_hertz(value); }}}
    , LinearDampingRatio {{[this]() -> f32 { return get_impl().weld_joint_get_linear_damping_ratio(); },
                           [this](auto const& value) { get_impl().weld_joint_set_linear_damping_ratio(value); }}}
    , AngularDampingRatio {{[this]() -> f32 { return get_impl().weld_joint_get_angular_damping_ratio(); },
                            [this](auto const& value) { get_impl().weld_joint_set_angular_damping_ratio(value); }}}
{
}

////////////////////////////////////////////////////////////

wheel_joint::wheel_joint(world& world, detail::b2d_world* b2dWorld, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(b2dWorld, get_body_impl(jointSettings.BodyA), get_body_impl(jointSettings.BodyB), jointSettings)}
    , EnableSpring {{[this]() -> bool { return get_impl().wheel_joint_is_spring_enabled(); },
                     [this](auto const& value) { get_impl().wheel_joint_enable_spring(value); }}}
    , Hertz {{[this]() -> f32 { return get_impl().wheel_joint_get_spring_hertz(); },
              [this](auto const& value) { get_impl().wheel_joint_set_spring_hertz(value); }}}
    , DampingRatio {{[this]() -> f32 { return get_impl().wheel_joint_get_spring_damping_ratio(); },
                     [this](auto const& value) { get_impl().wheel_joint_set_spring_damping_ratio(value); }}}
    , EnableLimit {{[this]() -> bool { return get_impl().wheel_joint_is_limit_enabled(); },
                    [this](auto const& value) { get_impl().wheel_joint_enable_limit(value); }}}
    , LowerTranslation {{[this]() -> f32 { return get_impl().wheel_joint_get_lower_limit(); },
                         [this](auto const& value) { get_impl().wheel_joint_set_limits(value, get_impl().wheel_joint_get_upper_limit()); }}}
    , UpperTranslation {{[this]() -> f32 { return get_impl().wheel_joint_get_upper_limit(); },
                         [this](auto const& value) { get_impl().wheel_joint_set_limits(get_impl().wheel_joint_get_lower_limit(), value); }}}
    , EnableMotor {{[this]() -> bool { return get_impl().wheel_joint_is_motor_enabled(); },
                    [this](auto const& value) { get_impl().wheel_joint_enable_motor(value); }}}
    , MaxMotorTorque {{[this]() -> f32 { return get_impl().wheel_joint_get_max_motor_torque(); },
                       [this](auto const& value) { get_impl().wheel_joint_set_max_motor_torque(value); }}}
    , MotorSpeed {{[this]() -> f32 { return get_impl().wheel_joint_get_motor_speed(); },
                   [this](auto const& value) { get_impl().wheel_joint_set_motor_speed(value); }}}
{
}

auto wheel_joint::motor_torque() const -> f32
{
    return get_impl().wheel_joint_get_motor_torque();
}

}

#endif
