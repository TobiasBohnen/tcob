// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/Joint.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <box2d/box2d.h>

    #include "B2D.hpp"

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

auto joint::get_impl() const -> detail::b2d_joint&
{
    return *_impl;
}

////////////////////////////////////////////////////////////

distance_joint::distance_joint(world& world, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(detail::get_impl(world), detail::get_impl(*jointSettings.BodyA), detail::get_impl(*jointSettings.BodyB), jointSettings)}
    , Length {{[&]() -> f32 { return get_impl().distance_joint_get_length(); },
               [&](auto const& value) { get_impl().distance_joint_set_length(value); }}}
    , EnableSpring {{[&]() -> bool { return get_impl().distance_joint_is_spring_enabled(); },
                     [&](auto const& value) { get_impl().distance_joint_enable_spring(value); }}}
    , Hertz {{[&]() -> f32 { return get_impl().distance_joint_get_hertz(); },
              [&](auto const& value) { get_impl().distance_joint_set_spring_hertz(value); }}}
    , DampingRatio {{[&]() -> f32 { return get_impl().distance_joint_get_damping_ratio(); },
                     [&](auto const& value) { get_impl().distance_joint_set_spring_damping_ratio(value); }}}
    , EnableLimit {{[&]() -> bool { return get_impl().distance_joint_is_limit_enabled(); },
                    [&](auto const& value) { get_impl().distance_joint_enable_limit(value); }}}
    , MinLength {{[&]() -> f32 { return get_impl().distance_joint_get_min_length(); },
                  [&](auto const& value) { get_impl().distance_joint_set_length_range(value, get_impl().distance_joint_get_max_length()); }}}
    , MaxLength {{[&]() -> f32 { return get_impl().distance_joint_get_max_length(); },
                  [&](auto const& value) { get_impl().distance_joint_set_length_range(get_impl().distance_joint_get_min_length(), value); }}}
    , EnableMotor {{[&]() -> bool { return get_impl().distance_joint_is_motor_enabled(); },
                    [&](auto const& value) { get_impl().distance_joint_enable_motor(value); }}}
    , MotorSpeed {{[&]() -> f32 { return get_impl().distance_joint_get_motor_speed(); },
                   [&](auto const& value) { get_impl().distance_joint_set_motor_speed(value); }}}
    , MaxMotorForce {{[&]() -> f32 { return get_impl().distance_joint_get_max_motor_force(); },
                      [&](auto const& value) { get_impl().distance_joint_set_motor_speed(value); }}}
{
}

auto distance_joint::get_current_length() const -> f32
{
    return get_impl().distance_joint_get_current_length();
}

auto distance_joint::get_motor_force() const -> f32
{
    return get_impl().distance_joint_get_motor_force();
}

////////////////////////////////////////////////////////////

motor_joint::motor_joint(world& world, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(detail::get_impl(world), detail::get_impl(*jointSettings.BodyA), detail::get_impl(*jointSettings.BodyB), jointSettings)}
    , LinearOffset {{[&]() -> point_f { return get_impl().motor_joint_get_linear_offset(); },
                     [&](auto const& value) { get_impl().motor_joint_set_linear_offset(value); }}}
    , AngularOffset {{[&]() -> f32 { return get_impl().motor_joint_get_angular_offset(); },
                      [&](auto const& value) { get_impl().motor_joint_set_angular_offset(value); }}}
    , MaxForce {{[&]() -> f32 { return get_impl().motor_joint_get_max_force(); },
                 [&](auto const& value) { get_impl().motor_joint_set_max_force(value); }}}
    , MaxTorque {{[&]() -> f32 { return get_impl().motor_joint_get_max_torque(); },
                  [&](auto const& value) { get_impl().motor_joint_set_max_torque(value); }}}
    , CorrectionFactor {{[&]() -> f32 { return get_impl().motor_joint_get_correction_factor(); },
                         [&](auto const& value) { get_impl().motor_joint_set_correction_factor(value); }}}
{
}

////////////////////////////////////////////////////////////

mouse_joint::mouse_joint(world& world, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(detail::get_impl(world), detail::get_impl(*jointSettings.BodyA), detail::get_impl(*jointSettings.BodyB), jointSettings)}
    , Target {{[&]() -> point_f { return get_impl().mouse_joint_get_target(); },
               [&](auto const& value) { get_impl().mouse_joint_set_target(value); }}}
    , Hertz {{[&]() -> f32 { return get_impl().mouse_joint_get_spring_hertz(); },
              [&](auto const& value) { get_impl().mouse_joint_set_spring_hertz(value); }}}
    , DampingRatio {{[&]() -> f32 { return get_impl().mouse_joint_get_spring_damping_ratio(); },
                     [&](auto const& value) { get_impl().mouse_joint_set_spring_damping_ratio(value); }}}
    , MaxForce {{[&]() -> f32 { return get_impl().mouse_joint_get_max_force(); },
                 [&](auto const& value) { get_impl().mouse_joint_set_max_force(value); }}}
{
}

////////////////////////////////////////////////////////////

prismatic_joint::prismatic_joint(world& world, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(detail::get_impl(world), detail::get_impl(*jointSettings.BodyA), detail::get_impl(*jointSettings.BodyB), jointSettings)}
    , EnableSpring {{[&]() -> bool { return get_impl().prismatic_joint_is_spring_enabled(); },
                     [&](auto const& value) { get_impl().prismatic_joint_enable_spring(value); }}}
    , Hertz {{[&]() -> f32 { return get_impl().prismatic_joint_get_spring_hertz(); },
              [&](auto const& value) { get_impl().prismatic_joint_set_spring_hertz(value); }}}
    , DampingRatio {{[&]() -> f32 { return get_impl().prismatic_joint_get_spring_damping_ratio(); },
                     [&](auto const& value) { get_impl().prismatic_joint_set_spring_damping_ratio(value); }}}
    , EnableLimit {{[&]() -> bool { return get_impl().prismatic_joint_is_limit_enabled(); },
                    [&](auto const& value) { get_impl().prismatic_joint_enable_limit(value); }}}
    , LowerTranslation {{[&]() -> f32 { return get_impl().prismatic_joint_get_lower_limit(); },
                         [&](auto const& value) { get_impl().prismatic_joint_set_limits(value, get_impl().prismatic_joint_get_upper_limit()); }}}
    , UpperTranslation {{[&]() -> f32 { return get_impl().prismatic_joint_get_upper_limit(); },
                         [&](auto const& value) { get_impl().prismatic_joint_set_limits(get_impl().prismatic_joint_get_lower_limit(), value); }}}
    , EnableMotor {{[&]() -> bool { return get_impl().prismatic_joint_is_motor_enabled(); },
                    [&](auto const& value) { get_impl().prismatic_joint_enable_motor(value); }}}
    , MaxMotorForce {{[&]() -> f32 { return get_impl().prismatic_joint_get_max_motor_force(); },
                      [&](auto const& value) { get_impl().prismatic_joint_set_max_motor_force(value); }}}
    , MotorSpeed {{[&]() -> f32 { return get_impl().prismatic_joint_get_motor_speed(); },
                   [&](auto const& value) { get_impl().prismatic_joint_set_motor_speed(value); }}}
{
}

auto prismatic_joint::get_motor_force() const -> f32
{
    return get_impl().prismatic_joint_get_motor_force();
}

////////////////////////////////////////////////////////////

revolute_joint::revolute_joint(world& world, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(detail::get_impl(world), detail::get_impl(*jointSettings.BodyA), detail::get_impl(*jointSettings.BodyB), jointSettings)}
    , EnableSpring {{[&]() -> bool { return get_impl().revolute_joint_is_spring_enabled(); },
                     [&](auto const& value) { get_impl().revolute_joint_enable_spring(value); }}}
    , Hertz {{[&]() -> f32 { return get_impl().revolute_joint_get_spring_hertz(); },
              [&](auto const& value) { get_impl().revolute_joint_set_spring_hertz(value); }}}
    , DampingRatio {{[&]() -> f32 { return get_impl().revolute_joint_get_spring_damping_ratio(); },
                     [&](auto const& value) { get_impl().revolute_joint_set_spring_damping_ratio(value); }}}
    , EnableLimit {{[&]() -> bool { return get_impl().revolute_joint_is_limit_enabled(); },
                    [&](auto const& value) { get_impl().revolute_joint_enable_limit(value); }}}
    , LowerAngle {{[&]() -> f32 { return get_impl().revolute_joint_get_lower_limit(); },
                   [&](auto const& value) { get_impl().revolute_joint_set_limits(value, get_impl().revolute_joint_get_upper_limit()); }}}
    , UpperAngle {{[&]() -> f32 { return get_impl().revolute_joint_get_upper_limit(); },
                   [&](auto const& value) { get_impl().revolute_joint_set_limits(get_impl().revolute_joint_get_lower_limit(), value); }}}
    , EnableMotor {{[&]() -> bool { return get_impl().revolute_joint_is_motor_enabled(); },
                    [&](auto const& value) { get_impl().revolute_joint_enable_motor(value); }}}
    , MaxMotorTorque {{[&]() -> f32 { return get_impl().revolute_joint_get_max_motor_torque(); },
                       [&](auto const& value) { get_impl().revolute_joint_set_max_motor_torque(value); }}}
    , MotorSpeed {{[&]() -> f32 { return get_impl().revolute_joint_get_motor_speed(); },
                   [&](auto const& value) { get_impl().revolute_joint_set_motor_speed(value); }}}
{
}

auto revolute_joint::get_angle() const -> radian_f
{
    return get_impl().revolute_joint_get_angle();
}

auto revolute_joint::get_motor_torque() const -> f32
{
    return get_impl().revolute_joint_get_motor_torque();
}

////////////////////////////////////////////////////////////

weld_joint::weld_joint(world& world, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(detail::get_impl(world), detail::get_impl(*jointSettings.BodyA), detail::get_impl(*jointSettings.BodyB), jointSettings)}
    , LinearHertz {{[&]() -> f32 { return get_impl().weld_joint_get_linear_hertz(); },
                    [&](auto const& value) { get_impl().weld_joint_set_linear_hertz(value); }}}
    , AngularHertz {{[&]() -> f32 { return get_impl().weld_joint_get_angular_hertz(); },
                     [&](auto const& value) { get_impl().weld_joint_set_angular_hertz(value); }}}
    , LinearDampingRatio {{[&]() -> f32 { return get_impl().weld_joint_get_linear_damping_ratio(); },
                           [&](auto const& value) { get_impl().weld_joint_set_linear_damping_ratio(value); }}}
    , AngularDampingRatio {{[&]() -> f32 { return get_impl().weld_joint_get_angular_damping_ratio(); },
                            [&](auto const& value) { get_impl().weld_joint_set_angular_damping_ratio(value); }}}
{
}

////////////////////////////////////////////////////////////

wheel_joint::wheel_joint(world& world, settings const& jointSettings)
    : joint {world, std::make_unique<detail::b2d_joint>(detail::get_impl(world), detail::get_impl(*jointSettings.BodyA), detail::get_impl(*jointSettings.BodyB), jointSettings)}
    , EnableSpring {{[&]() -> bool { return get_impl().wheel_joint_is_spring_enabled(); },
                     [&](auto const& value) { get_impl().wheel_joint_enable_spring(value); }}}
    , Hertz {{[&]() -> f32 { return get_impl().wheel_joint_get_spring_hertz(); },
              [&](auto const& value) { get_impl().wheel_joint_set_spring_hertz(value); }}}
    , DampingRatio {{[&]() -> f32 { return get_impl().wheel_joint_get_spring_damping_ratio(); },
                     [&](auto const& value) { get_impl().wheel_joint_set_spring_damping_ratio(value); }}}
    , EnableLimit {{[&]() -> bool { return get_impl().wheel_joint_is_limit_enabled(); },
                    [&](auto const& value) { get_impl().wheel_joint_enable_limit(value); }}}
    , LowerTranslation {{[&]() -> f32 { return get_impl().wheel_joint_get_lower_limit(); },
                         [&](auto const& value) { get_impl().wheel_joint_set_limits(value, get_impl().wheel_joint_get_upper_limit()); }}}
    , UpperTranslation {{[&]() -> f32 { return get_impl().wheel_joint_get_upper_limit(); },
                         [&](auto const& value) { get_impl().wheel_joint_set_limits(get_impl().wheel_joint_get_lower_limit(), value); }}}
    , EnableMotor {{[&]() -> bool { return get_impl().wheel_joint_is_motor_enabled(); },
                    [&](auto const& value) { get_impl().wheel_joint_enable_motor(value); }}}
    , MaxMotorTorque {{[&]() -> f32 { return get_impl().wheel_joint_get_max_motor_torque(); },
                       [&](auto const& value) { get_impl().wheel_joint_set_max_motor_torque(value); }}}
    , MotorSpeed {{[&]() -> f32 { return get_impl().wheel_joint_get_motor_speed(); },
                   [&](auto const& value) { get_impl().wheel_joint_set_motor_speed(value); }}}
{
}

auto wheel_joint::get_motor_torque() const -> f32
{
    return get_impl().wheel_joint_get_motor_torque();
}

} // namespace box2d

#endif
