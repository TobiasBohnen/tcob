// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/box2d/B2DWorld.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <box2d/box2d.h>

    #include <memory>

    #include "B2DContactListener.hpp"

namespace tcob::physics::box2d {

world::world()
    : _b2World {std::make_unique<b2World>(b2Vec2_zero)}
{
    Gravity.Changed.connect([&](point_f value) {
        _b2World->SetGravity({value.X, value.Y});
    });
    Gravity(point_f::Zero);

    AllowSleeping.Changed.connect([&](bool value) {
        _b2World->SetAllowSleeping(value);
    });
    AllowSleeping(true);

    _listener = std::make_unique<detail::contact_listener>(this);
    _b2World->SetContactListener(_listener.get());
}

world::~world() = default;

auto world::get_bodies() -> std::span<std::shared_ptr<body>>
{
    return _bodies;
}

auto world::get_joints() -> std::span<std::shared_ptr<joint>>
{
    return _joints;
}

auto world::create_body(body_transform const& xform, body_settings const& bodySettings) -> std::shared_ptr<body>
{
    assert(_b2World);

    b2BodyDef def;
    switch (bodySettings.Type) {
    case body_type::Dynamic:
        def.type = b2_dynamicBody;
        break;
    case body_type::Static:
        def.type = b2_staticBody;
        break;
    case body_type::Kinematic:
        def.type = b2_kinematicBody;
        break;
    }
    def.position        = {xform.Position.X, xform.Position.Y};
    def.angle           = xform.Angle.Value;
    def.linearVelocity  = {bodySettings.LinearVelocity.X, bodySettings.LinearVelocity.Y};
    def.angularVelocity = bodySettings.AngularVelocity.Value;
    def.linearDamping   = bodySettings.LinearDamping;
    def.angularDamping  = bodySettings.AngularDamping;
    def.allowSleep      = bodySettings.AllowSleep;
    def.fixedRotation   = bodySettings.IsFixedRotation;
    def.bullet          = bodySettings.IsBullet;
    def.enabled         = bodySettings.IsEnabled;
    def.gravityScale    = bodySettings.GravityScale;

    return _bodies.emplace_back(std::shared_ptr<body> {new body {_b2World->CreateBody(&def), this}});
}

void world::destroy_body(std::shared_ptr<body> const& bodyPtr)
{
    _bodies.erase(std::find(_bodies.begin(), _bodies.end(), bodyPtr));
    _b2World->DestroyBody(detail::get_impl(bodyPtr.get()));
}

auto world::create_joint(distance_joint_settings const& jointSettings) -> std::shared_ptr<distance_joint>
{
    assert(_b2World);

    b2DistanceJointDef def;
    def.bodyA            = detail::get_impl(jointSettings.BodyA.get());
    def.bodyB            = detail::get_impl(jointSettings.BodyB.get());
    def.collideConnected = jointSettings.IsCollideConnected;
    def.damping          = jointSettings.Damping;
    def.length           = jointSettings.Length;
    def.localAnchorA     = {jointSettings.LocalAnchorA.X, jointSettings.LocalAnchorA.Y};
    def.localAnchorB     = {jointSettings.LocalAnchorB.X, jointSettings.LocalAnchorB.Y};
    def.maxLength        = jointSettings.MaxLength;
    def.minLength        = jointSettings.MinLength;
    def.stiffness        = jointSettings.Stiffness;

    ///
    auto* jointPtr {_b2World->CreateJoint(&def)};
    auto  retValue {std::make_shared<distance_joint>(jointPtr, this)};
    _joints.push_back(retValue);

    return retValue;
}

auto world::create_joint(friction_joint_settings const& jointSettings) -> std::shared_ptr<friction_joint>
{
    assert(_b2World);

    b2FrictionJointDef def;
    def.bodyA            = detail::get_impl(jointSettings.BodyA.get());
    def.bodyB            = detail::get_impl(jointSettings.BodyB.get());
    def.collideConnected = jointSettings.IsCollideConnected;
    def.localAnchorA     = {jointSettings.LocalAnchorA.X, jointSettings.LocalAnchorA.Y};
    def.localAnchorB     = {jointSettings.LocalAnchorB.X, jointSettings.LocalAnchorB.Y};
    def.maxForce         = jointSettings.MaxForce;
    def.maxTorque        = jointSettings.MaxTorque;

    ///
    auto* jointPtr {_b2World->CreateJoint(&def)};
    auto  retValue {std::make_shared<friction_joint>(jointPtr, this)};
    _joints.push_back(retValue);

    return retValue;
}

auto world::create_joint(gear_joint_settings const& jointSettings) -> std::shared_ptr<gear_joint>
{
    assert(_b2World);

    b2GearJointDef def;
    def.bodyA            = detail::get_impl(jointSettings.BodyA.get());
    def.bodyB            = detail::get_impl(jointSettings.BodyB.get());
    def.collideConnected = jointSettings.IsCollideConnected;
    def.joint1           = detail::get_impl(jointSettings.Joint1.get());
    def.joint2           = detail::get_impl(jointSettings.Joint2.get());
    def.ratio            = jointSettings.Ratio;

    ///
    auto* jointPtr {_b2World->CreateJoint(&def)};
    auto  retValue {std::make_shared<gear_joint>(jointPtr, this)};
    _joints.push_back(retValue);

    return retValue;
}

auto world::create_joint(motor_joint_settings const& jointSettings) -> std::shared_ptr<motor_joint>
{
    assert(_b2World);

    b2MotorJointDef def;
    def.bodyA            = detail::get_impl(jointSettings.BodyA.get());
    def.bodyB            = detail::get_impl(jointSettings.BodyB.get());
    def.collideConnected = jointSettings.IsCollideConnected;
    def.angularOffset    = jointSettings.AngularOffset.Value;
    def.correctionFactor = jointSettings.CorrectionFactor;
    def.linearOffset     = {jointSettings.LinearOffset.X, jointSettings.LinearOffset.Y};
    def.maxForce         = jointSettings.MaxForce;
    def.maxTorque        = jointSettings.MaxTorque;

    ///
    auto* jointPtr {_b2World->CreateJoint(&def)};
    auto  retValue {std::make_shared<motor_joint>(jointPtr, this)};
    _joints.push_back(retValue);

    return retValue;
}

auto world::create_joint(mouse_joint_settings const& jointSettings) -> std::shared_ptr<mouse_joint>
{
    assert(_b2World);

    b2MouseJointDef def;
    def.bodyA            = detail::get_impl(jointSettings.BodyA.get());
    def.bodyB            = detail::get_impl(jointSettings.BodyB.get());
    def.collideConnected = jointSettings.IsCollideConnected;
    def.damping          = jointSettings.Damping;
    def.maxForce         = jointSettings.MaxForce;
    def.stiffness        = jointSettings.Stiffness;
    def.target           = {jointSettings.Target.X, jointSettings.Target.Y};

    ///
    auto* jointPtr {_b2World->CreateJoint(&def)};
    auto  retValue {std::make_shared<mouse_joint>(jointPtr, this)};
    _joints.push_back(retValue);

    return retValue;
}

auto world::create_joint(prismatic_joint_settings const& jointSettings) -> std::shared_ptr<prismatic_joint>
{
    assert(_b2World);

    b2PrismaticJointDef def;
    def.bodyA            = detail::get_impl(jointSettings.BodyA.get());
    def.bodyB            = detail::get_impl(jointSettings.BodyB.get());
    def.collideConnected = jointSettings.IsCollideConnected;
    def.enableLimit      = jointSettings.IsLimitEnabled;
    def.enableMotor      = jointSettings.IsMotorEnabled;
    def.localAnchorA     = {jointSettings.LocalAnchorA.X, jointSettings.LocalAnchorA.Y};
    def.localAnchorB     = {jointSettings.LocalAnchorB.X, jointSettings.LocalAnchorB.Y};
    def.localAxisA       = {jointSettings.LocalAxisA.X, jointSettings.LocalAxisA.Y};
    def.lowerTranslation = jointSettings.LowerTranslation;
    def.maxMotorForce    = jointSettings.MaxMotorForce;
    def.motorSpeed       = jointSettings.MotorSpeed.Value;
    def.referenceAngle   = jointSettings.ReferenceAngle.Value;
    def.upperTranslation = jointSettings.UpperTranslation;

    ///
    auto* jointPtr {_b2World->CreateJoint(&def)};
    auto  retValue {std::make_shared<prismatic_joint>(jointPtr, this)};
    _joints.push_back(retValue);

    return retValue;
}

auto world::create_joint(pulley_joint_settings const& jointSettings) -> std::shared_ptr<pulley_joint>
{
    assert(_b2World);

    b2PulleyJointDef def;
    def.bodyA            = detail::get_impl(jointSettings.BodyA.get());
    def.bodyB            = detail::get_impl(jointSettings.BodyB.get());
    def.collideConnected = jointSettings.IsCollideConnected;
    def.groundAnchorA    = {jointSettings.GroundAnchorA.X, jointSettings.GroundAnchorA.Y};
    def.groundAnchorB    = {jointSettings.GroundAnchorB.X, jointSettings.GroundAnchorB.Y};
    def.lengthA          = jointSettings.LengthA;
    def.lengthB          = jointSettings.LengthB;
    def.localAnchorA     = {jointSettings.LocalAnchorA.X, jointSettings.LocalAnchorA.Y};
    def.localAnchorB     = {jointSettings.LocalAnchorB.X, jointSettings.LocalAnchorB.Y};
    def.ratio            = jointSettings.Ratio;

    ///
    auto* jointPtr {_b2World->CreateJoint(&def)};
    auto  retValue {std::make_shared<pulley_joint>(jointPtr, this)};
    _joints.push_back(retValue);

    return retValue;
}

auto world::create_joint(revolute_joint_settings const& jointSettings) -> std::shared_ptr<revolute_joint>
{
    assert(_b2World);

    b2RevoluteJointDef def;
    def.bodyA            = detail::get_impl(jointSettings.BodyA.get());
    def.bodyB            = detail::get_impl(jointSettings.BodyB.get());
    def.collideConnected = jointSettings.IsCollideConnected;
    def.enableLimit      = jointSettings.IsLimitEnabled;
    def.enableMotor      = jointSettings.IsMotorEnabled;
    def.localAnchorA     = {jointSettings.LocalAnchorA.X, jointSettings.LocalAnchorA.Y};
    def.localAnchorB     = {jointSettings.LocalAnchorB.X, jointSettings.LocalAnchorB.Y};
    def.lowerAngle       = jointSettings.LowerAngle.Value;
    def.maxMotorTorque   = jointSettings.MaxMotorTorque;
    def.motorSpeed       = jointSettings.MotorSpeed.Value;
    def.referenceAngle   = jointSettings.ReferenceAngle.Value;
    def.upperAngle       = jointSettings.UpperAngle.Value;

    ///
    auto* jointPtr {_b2World->CreateJoint(&def)};
    auto  retValue {std::make_shared<revolute_joint>(jointPtr, this)};
    _joints.push_back(retValue);

    return retValue;
}

auto world::create_joint(weld_joint_settings const& jointSettings) -> std::shared_ptr<weld_joint>
{
    assert(_b2World);

    b2WeldJointDef def;
    def.bodyA            = detail::get_impl(jointSettings.BodyA.get());
    def.bodyB            = detail::get_impl(jointSettings.BodyB.get());
    def.collideConnected = jointSettings.IsCollideConnected;
    def.damping          = jointSettings.Damping;
    def.localAnchorA     = {jointSettings.LocalAnchorA.X, jointSettings.LocalAnchorA.Y};
    def.localAnchorB     = {jointSettings.LocalAnchorB.X, jointSettings.LocalAnchorB.Y};
    def.referenceAngle   = jointSettings.ReferenceAngle.Value;
    def.stiffness        = jointSettings.Stiffness;

    ///
    auto* jointPtr {_b2World->CreateJoint(&def)};
    auto  retValue {std::make_shared<weld_joint>(jointPtr, this)};
    _joints.push_back(retValue);

    return retValue;
}

auto world::create_joint(wheel_joint_settings const& jointSettings) -> std::shared_ptr<wheel_joint>
{
    assert(_b2World);

    b2WheelJointDef def;
    def.bodyA            = detail::get_impl(jointSettings.BodyA.get());
    def.bodyB            = detail::get_impl(jointSettings.BodyB.get());
    def.collideConnected = jointSettings.IsCollideConnected;
    def.damping          = jointSettings.Damping;
    def.localAnchorA     = {jointSettings.LocalAnchorA.X, jointSettings.LocalAnchorA.Y};
    def.localAnchorB     = {jointSettings.LocalAnchorB.X, jointSettings.LocalAnchorB.Y};
    def.enableLimit      = jointSettings.IsLimitEnabled;
    def.enableMotor      = jointSettings.IsMotorEnabled;
    def.localAxisA       = {jointSettings.LocalAxisA.X, jointSettings.LocalAxisA.Y};
    def.lowerTranslation = jointSettings.LowerTranslation;
    def.maxMotorTorque   = jointSettings.MaxMotorTorque;
    def.motorSpeed       = jointSettings.MotorSpeed.Value;
    def.upperTranslation = jointSettings.UpperTranslation;
    def.stiffness        = jointSettings.Stiffness;

    ///
    auto* jointPtr {_b2World->CreateJoint(&def)};
    auto  retValue {std::make_shared<wheel_joint>(jointPtr, this)};
    _joints.push_back(retValue);

    return retValue;
}

void world::destroy_joint(std::shared_ptr<joint> const& jointPtr)
{
    _joints.erase(std::find(_joints.begin(), _joints.end(), jointPtr));
    _b2World->DestroyJoint(detail::get_impl(jointPtr.get()));
}

void world::do_debug_draw(debug_draw const& draw)
{
    _b2World->SetDebugDraw(detail::get_impl(&draw));
    _b2World->DebugDraw();
}

void world::on_update(milliseconds deltaTime)
{
    assert(_b2World);
    _b2World->Step(static_cast<f32>(deltaTime.count() / 1000), Iterations.Velocity, Iterations.Position);
}

auto world::is_locked() const -> bool
{
    assert(_b2World);
    return _b2World->IsLocked();
}

auto world::find_fixture(b2Fixture* b2fixture) -> std::shared_ptr<fixture>
{
    for (auto& body : _bodies) {
        if (detail::get_impl(body.get()) == b2fixture->GetBody()) {
            for (auto& fix : body->get_fixtures()) {
                if (detail::get_impl(fix.get()) == b2fixture) {
                    return fix;
                }
            }
        }
    }

    return nullptr;
}

}

#endif
