// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include "tcob/core/AngleUnits.hpp"
    #include "tcob/core/Interfaces.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/physics/Body.hpp"
    #include "tcob/physics/Physics.hpp"

namespace tcob::physics {
////////////////////////////////////////////////////////////

class TCOB_API joint_settings {
public:
    /// The first attached body.
    std::shared_ptr<body> BodyA;

    /// The second attached body.
    std::shared_ptr<body> BodyB;

    /// Set this flag to true if the attached bodies should collide.
    bool IsCollideConnected {false};
};

class TCOB_API joint : public non_copyable {
    friend class world;

public:
    ~joint();

protected:
    joint(std::unique_ptr<detail::b2d_joint> impl);

private:
    std::unique_ptr<detail::b2d_joint> _impl;
};

////////////////////////////////////////////////////////////

class TCOB_API distance_joint_settings : public joint_settings {
public:
    /// The local anchor point relative to bodyA's origin.
    point_f LocalAnchorA {point_f::Zero};

    /// The local anchor point relative to bodyB's origin.
    point_f LocalAnchorB {point_f::Zero};

    /// The rest length of this joint. Clamped to a stable minimum value.
    f32 Length {1.0f};

    /// Enable the distance constraint to behave like a spring. If false
    ///	then the distance joint will be rigid, overriding the limit and motor.
    bool EnableSpring {false};

    /// The spring linear stiffness Hertz, cycles per second
    f32 Hertz {0.0f};

    /// The spring linear damping ratio, non-dimensional
    f32 DampingRatio {0.0f};

    /// Enable/disable the joint limit
    bool EnableLimit {false};

    /// Minimum length. Clamped to a stable minimum value.
    f32 MinLength {0.0f};

    /// Maximum length. Must be greater than or equal to the minimum length.
    f32 MaxLength {100000.0f};

    /// Enable/disable the joint motor
    bool EnableMotor {false};

    /// The maximum motor force, usually in newtons
    f32 MaxMotorForce {0.0f};

    /// The desired motor speed, usually in meters per second
    f32 MotorSpeed {0.0f};
};

class TCOB_API distance_joint final : public joint {
    friend class world;

private:
    distance_joint(detail::b2d_world* world, distance_joint_settings const& jointSettings);
};

////////////////////////////////////////////////////////////

class TCOB_API motor_joint_settings : public joint_settings {
public:
    /// Position of bodyB minus the position of bodyA, in bodyA's frame, in meters.
    point_f LinearOffset {point_f::Zero};

    /// The bodyB angle minus bodyA angle in radians.
    radian_f AngularOffset {0};

    /// The maximum motor force in N.
    f32 MaxForce {1.0f};

    /// The maximum motor torque in N-m.
    f32 MaxTorque {1.0f};

    /// Position correction factor in the range [0,1].
    f32 CorrectionFactor {0.3f};
};

class TCOB_API motor_joint final : public joint {
    friend class world;

private:
    motor_joint(detail::b2d_world* world, motor_joint_settings const& jointSettings);
};

////////////////////////////////////////////////////////////

struct mouse_joint_settings : public joint_settings {

    /// The initial target point in world space
    point_f Target;

    /// Stiffness in hertz
    f32 Hertz {4.0f};

    /// Damping ratio, non-dimensional
    f32 DampingRatio {1.0f};

    /// Maximum force, typically in newtons
    f32 MaxForce {1.0f};
};

class TCOB_API mouse_joint final : public joint {
    friend class world;

private:
    mouse_joint(detail::b2d_world* world, mouse_joint_settings const& jointSettings);
};

////////////////////////////////////////////////////////////

class TCOB_API prismatic_joint_settings : public joint_settings {
public:
    /// The local anchor point relative to bodyA's origin.
    point_f LocalAnchorA {point_f::Zero};

    /// The local anchor point relative to bodyB's origin.
    point_f LocalAnchorB {point_f::Zero};

    /// The local translation unit axis in bodyA.
    point_f LocalAxisA {1.0f, 0.0f};

    /// The constrained angle between the bodies: bodyB_angle - bodyA_angle.
    radian_f ReferenceAngle {0};

    /// Enable a linear spring along the prismatic joint axis
    bool EnableSpring {false};

    /// The spring stiffness Hertz, cycles per second
    f32 Hertz {0.0f};

    /// The spring damping ratio, non-dimensional
    f32 DampingRatio {0.0f};

    /// Enable/disable the joint limit
    bool EnableLimit {false};

    /// The lower translation limit
    f32 LowerTranslation {0.0f};

    /// The upper translation limit
    f32 UpperTranslation {0.0f};

    /// Enable/disable the joint motor
    bool EnableMotor {false};

    /// The maximum motor force, typically in newtons
    f32 MaxMotorForce {0.0f};

    /// The desired motor speed, typically in meters per second
    f32 MotorSpeed {0.0f};
};

class TCOB_API prismatic_joint final : public joint {
    friend class world;

private:
    prismatic_joint(detail::b2d_world* world, prismatic_joint_settings const& jointSettings);
};

////////////////////////////////////////////////////////////

class TCOB_API revolute_joint_settings : public joint_settings {
public:
    /// The local anchor point relative to bodyA's origin.
    point_f LocalAnchorA {point_f::Zero};

    /// The local anchor point relative to bodyB's origin.
    point_f LocalAnchorB {point_f::Zero};

    /// The bodyB angle minus bodyA angle in the reference state (radians).
    radian_f ReferenceAngle {0};

    /// Enable a rotational spring on the revolute hinge axis
    bool EnableSpring {false};

    /// The spring stiffness Hertz, cycles per second
    f32 Hertz {0.0f};

    /// The spring damping ratio, non-dimensional
    f32 DampingRatio {0.0f};

    /// A flag to enable joint limits
    bool EnableLimit {false};

    /// The lower angle for the joint limit in radians
    f32 LowerAngle {0.0f};

    /// The upper angle for the joint limit in radians
    f32 UpperAngle {0.0f};

    /// A flag to enable the joint motor
    bool EnableMotor {false};

    /// The maximum motor torque, typically in newton-meters
    f32 MaxMotorTorque {0.0f};

    /// The desired motor speed in radians per second
    f32 MotorSpeed {0.0f};

    /// Scale the debug draw
    f32 DrawSize {0.25f};
};

class TCOB_API revolute_joint final : public joint {
    friend class world;

private:
    revolute_joint(detail::b2d_world* world, revolute_joint_settings const& jointSettings);
};

////////////////////////////////////////////////////////////

class TCOB_API weld_joint_settings : public joint_settings {
public:
    /// The local anchor point relative to bodyA's origin.
    point_f LocalAnchorA {point_f::Zero};

    /// The local anchor point relative to bodyB's origin.
    point_f LocalAnchorB {point_f::Zero};

    /// The bodyB angle minus bodyA angle in the reference state (degrees).
    radian_f ReferenceAngle {0};

    /// Linear stiffness expressed as Hertz (cycles per second). Use zero for maximum stiffness.
    f32 LinearHertz {0.0f};

    /// Angular stiffness as Hertz (cycles per second). Use zero for maximum stiffness.
    f32 AngularHertz {0.0f};

    /// Linear damping ratio, non-dimensional. Use 1 for critical damping.
    f32 LinearDampingRatio {0.0f};

    /// Linear damping ratio, non-dimensional. Use 1 for critical damping.
    f32 AngularDampingRatio {0.0f};
};

class TCOB_API weld_joint final : public joint {
    friend class world;

private:
    weld_joint(detail::b2d_world* world, weld_joint_settings const& jointSettings);
};

////////////////////////////////////////////////////////////

class TCOB_API wheel_joint_settings : public joint_settings {
public:
    /// The local anchor point relative to bodyA's origin.
    point_f LocalAnchorA {point_f::Zero};

    /// The local anchor point relative to bodyB's origin.
    point_f LocalAnchorB {point_f::Zero};

    /// The local translation axis in bodyA.
    point_f LocalAxisA {0.0f, 1.0f};

    /// Enable a linear spring along the local axis
    bool EnableSpring {true};

    /// Spring stiffness in Hertz
    f32 Hertz {1.0f};

    /// Spring damping ratio, non-dimensional
    f32 DampingRatio {0.7f};

    /// Enable/disable the joint linear limit
    bool EnableLimit {false};

    /// The lower translation limit
    f32 LowerTranslation {0.0f};

    /// The upper translation limit
    f32 UpperTranslation {0.0f};

    /// Enable/disable the joint rotational motor
    bool EnableMotor {false};

    /// The maximum motor torque, typically in newton-meters
    f32 MaxMotorTorque {0.0f};

    /// The desired motor speed in radians per second
    f32 MotorSpeed {0.0f};
};

class TCOB_API wheel_joint final : public joint {
    friend class world;

private:
    wheel_joint(detail::b2d_world* world, wheel_joint_settings const& jointSettings);
};

}

#endif
