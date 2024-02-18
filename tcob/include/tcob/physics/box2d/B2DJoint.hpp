// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include "tcob/core/AngleUnits.hpp"
    #include "tcob/core/Interfaces.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/physics/box2d/B2D.hpp"
    #include "tcob/physics/box2d/B2DBody.hpp"

namespace tcob::physics::box2d {
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
    friend auto detail::get_impl(joint const* b) -> b2Joint*;
    friend class world;

public:
    ~joint() = default;

protected:
    joint(b2Joint* b2joint, world* world);

    template <typename T>
    auto get_impl() const -> T*
    {
        return static_cast<T*>(_b2Joint);
    }

    auto get_world() const -> world&;

private:
    b2Joint* _b2Joint;
    world*   _world;
};

////////////////////////////////////////////////////////////

class TCOB_API distance_joint_settings : public joint_settings {
public:
    /// Initialize the bodies, anchors, and rest length using world space anchors.
    /// The minimum and maximum lengths are set to the rest length.
    void initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB, point_f anchorA, point_f anchorB);

    /// The local anchor point relative to bodyA's origin.
    point_f LocalAnchorA {point_f::Zero};

    /// The local anchor point relative to bodyB's origin.
    point_f LocalAnchorB {point_f::Zero};

    /// The rest length of this joint. Clamped to a stable minimum value.
    f32 Length {1.0f};

    /// Minimum length. Clamped to a stable minimum value.
    f32 MinLength {0.0f};

    /// Maximum length. Must be greater than or equal to the minimum length.
    f32 MaxLength {std::numeric_limits<f32>::max()};

    /// The linear stiffness in N/m.
    f32 Stiffness {0.0f};

    /// The linear damping in N*s/m.
    f32 Damping {0.0f};
};

class TCOB_API distance_joint final : public joint {
public:
    distance_joint(b2Joint* b2joint, world* world);
};

////////////////////////////////////////////////////////////

class TCOB_API friction_joint_settings : public joint_settings {
public:
    /// Initialize the bodies, anchors, axis, and reference angle using the world
    /// anchor and world axis.
    void initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB, point_f anchor);

    /// The local anchor point relative to bodyA's origin.
    point_f LocalAnchorA;

    /// The local anchor point relative to bodyB's origin.
    point_f LocalAnchorB;

    /// The maximum friction force in N.
    f32 MaxForce {};

    /// The maximum friction torque in N-m.
    f32 MaxTorque {};
};

class TCOB_API friction_joint final : public joint {
public:
    friction_joint(b2Joint* b2joint, world* world);
};

////////////////////////////////////////////////////////////

struct gear_joint_settings : public joint_settings {
    /// The first revolute/prismatic joint attached to the gear joint.
    std::shared_ptr<joint> Joint1;

    /// The second revolute/prismatic joint attached to the gear joint.
    std::shared_ptr<joint> Joint2;

    /// The gear ratio.
    /// @see b2GearJoint for explanation.
    f32 Ratio {1.0f};
};

class TCOB_API gear_joint final : public joint {
public:
    gear_joint(b2Joint* b2joint, world* world);
};

////////////////////////////////////////////////////////////

class TCOB_API motor_joint_settings : public joint_settings {
public:
    /// Initialize the bodies and offsets using the current transforms.
    void initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB);

    /// Position of bodyB minus the position of bodyA, in bodyA's frame, in meters.
    point_f LinearOffset {point_f::Zero};

    /// The bodyB angle minus bodyA angle in radians.
    radian_f AngularOffset {0.0f};

    /// The maximum motor force in N.
    f32 MaxForce {1.0f};

    /// The maximum motor torque in N-m.
    f32 MaxTorque {1.0f};

    /// Position correction factor in the range [0,1].
    f32 CorrectionFactor {0.3f};
};

class TCOB_API motor_joint final : public joint {
public:
    motor_joint(b2Joint* b2joint, world* world);
};

////////////////////////////////////////////////////////////

struct mouse_joint_settings : public joint_settings {

    /// The initial world target point. This is assumed
    /// to coincide with the body anchor initially.
    point_f Target {point_f::Zero};

    /// The maximum constraint force that can be exerted
    /// to move the candidate body. Usually you will express
    /// as some multiple of the weight (multiplier * mass * gravity).
    f32 MaxForce {0.0f};

    /// The linear stiffness in N/m
    f32 Stiffness {0.0f};

    /// The linear damping in N*s/m
    f32 Damping {0.0f};
};

class TCOB_API mouse_joint final : public joint {
public:
    mouse_joint(b2Joint* b2joint, world* world);
};

////////////////////////////////////////////////////////////

class TCOB_API prismatic_joint_settings : public joint_settings {
public:
    /// Initialize the bodies, anchors, axis, and reference angle using the world
    /// anchor and unit world axis.
    void initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB, point_f anchor, point_f axis);

    /// The local anchor point relative to bodyA's origin.
    point_f LocalAnchorA {point_f::Zero};

    /// The local anchor point relative to bodyB's origin.
    point_f LocalAnchorB {point_f::Zero};

    /// The local translation unit axis in bodyA.
    point_f LocalAxisA {1.0f, 0.0f};

    /// The constrained angle between the bodies: bodyB_angle - bodyA_angle.
    radian_f ReferenceAngle {0.0f};

    /// Enable/disable the joint limit.
    bool IsLimitEnabled {false};

    /// The lower translation limit, usually in meters.
    f32 LowerTranslation {0.0f};

    /// The upper translation limit, usually in meters.
    f32 UpperTranslation {0.0f};

    /// Enable/disable the joint motor.
    bool IsMotorEnabled {false};

    /// The maximum motor torque, usually in N-m.
    f32 MaxMotorForce {0.0f};

    /// The desired motor speed in radians per second.
    radian_f MotorSpeed {0.0f};
};

class TCOB_API prismatic_joint final : public joint {
public:
    prismatic_joint(b2Joint* b2joint, world* world);
};

////////////////////////////////////////////////////////////

class TCOB_API pulley_joint_settings : public joint_settings {
public:
    pulley_joint_settings();

    /// Initialize the bodies, anchors, lengths, max lengths, and ratio using the world anchors.
    void initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB,
                    point_f groundAnchorA, point_f groundAnchorB,
                    point_f anchorA, point_f anchorB,
                    f32 ratio);

    /// The first ground anchor in world coordinates. This point never moves.
    point_f GroundAnchorA {-1.0f, 1.0f};

    /// The second ground anchor in world coordinates. This point never moves.
    point_f GroundAnchorB {1.0f, 1.0f};

    /// The local anchor point relative to bodyA's origin.
    point_f LocalAnchorA {-1.0f, 0.0f};

    /// The local anchor point relative to bodyB's origin.
    point_f LocalAnchorB {1.0f, 0.0f};

    /// The a reference length for the segment attached to bodyA.
    f32 LengthA {0.0f};

    /// The a reference length for the segment attached to bodyB.
    f32 LengthB {0.0f};

    /// The pulley ratio, used to simulate a block-and-tackle.
    f32 Ratio {1.0f};
};

class TCOB_API pulley_joint final : public joint {
public:
    pulley_joint(b2Joint* b2joint, world* world);
};

////////////////////////////////////////////////////////////

class TCOB_API revolute_joint_settings : public joint_settings {
public:
    /// Initialize the bodies, anchors, and reference angle using a world
    /// anchor point.
    void initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB, point_f anchor);

    /// The local anchor point relative to bodyA's origin.
    point_f LocalAnchorA {point_f::Zero};

    /// The local anchor point relative to bodyB's origin.
    point_f LocalAnchorB {point_f::Zero};

    /// The bodyB angle minus bodyA angle in the reference state (radians).
    radian_f ReferenceAngle {0.0f};

    /// A flag to enable joint limits.
    bool IsLimitEnabled {false};

    /// The lower angle for the joint limit (radians).
    radian_f LowerAngle {0.0f};

    /// The upper angle for the joint limit (radians).
    radian_f UpperAngle {0.0f};

    /// A flag to enable the joint motor.
    bool IsMotorEnabled {false};

    /// The desired motor speed. Usually in radians per second.
    radian_f MotorSpeed {0.0f};

    /// The maximum motor torque used to achieve the desired motor speed.
    /// Usually in N-m.
    f32 MaxMotorTorque {0.0f};
};

class TCOB_API revolute_joint final : public joint {
public:
    revolute_joint(b2Joint* b2joint, world* world);
};

////////////////////////////////////////////////////////////

class TCOB_API weld_joint_settings : public joint_settings {
public:
    /// Initialize the bodies, anchors, reference angle, stiffness, and damping.
    /// @param bodyA the first body connected by this joint
    /// @param bodyB the second body connected by this joint
    /// @param anchor the point of connection in world coordinates
    void initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB, point_f anchor);

    /// The local anchor point relative to bodyA's origin.
    point_f LocalAnchorA {point_f::Zero};

    /// The local anchor point relative to bodyB's origin.
    point_f LocalAnchorB {point_f::Zero};

    /// The bodyB angle minus bodyA angle in the reference state (degrees).
    radian_f ReferenceAngle {0.0f};

    /// The rotational stiffness in N*m
    /// Disable softness with a value of 0
    f32 Stiffness {0.0f};

    /// The rotational damping in N*m*s
    f32 Damping {0.0f};
};

class TCOB_API weld_joint final : public joint {
public:
    weld_joint(b2Joint* b2joint, world* world);
};

////////////////////////////////////////////////////////////

class TCOB_API wheel_joint_settings : public joint_settings {
public:
    /// Initialize the bodies, anchors, axis, and reference angle using the world
    /// anchor and world axis.
    void initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB, point_f anchor, point_f axis);

    /// The local anchor point relative to bodyA's origin.
    point_f LocalAnchorA {point_f::Zero};

    /// The local anchor point relative to bodyB's origin.
    point_f LocalAnchorB {point_f::Zero};

    /// The local translation axis in bodyA.
    point_f LocalAxisA {1.0f, 0.0f};

    /// Enable/disable the joint limit.
    bool IsLimitEnabled {false};

    /// The lower translation limit, usually in meters.
    f32 LowerTranslation {0.0f};

    /// The upper translation limit, usually in meters.
    f32 UpperTranslation {0.0f};

    /// Enable/disable the joint motor.
    bool IsMotorEnabled {false};

    /// The maximum motor torque, usually in N-m.
    f32 MaxMotorTorque {0.0f};

    /// The desired motor speed in radians per second.
    radian_f MotorSpeed {0.0f};

    /// Suspension stiffness. Typically in units N/m.
    f32 Stiffness {0.0f};

    /// Suspension damping. Typically in units of N*s/m.
    f32 Damping {0.0f};
};

class TCOB_API wheel_joint final : public joint {
public:
    wheel_joint(b2Joint* b2joint, world* world);
};

////////////////////////////////////////////////////////////

inline auto operator==(joint const& left, joint const& right) -> bool
{
    return detail::get_impl(&left) == detail::get_impl(&right);
}

}

#endif
