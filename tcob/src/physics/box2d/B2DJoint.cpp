// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/box2d/B2DJoint.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <box2d/box2d.h>

    #include "tcob/physics/box2d/B2DWorld.hpp"

namespace tcob::physics::box2d {

joint::joint(b2Joint* b2joint, world* world)
    : _b2Joint {b2joint}
    , _world {world}
{
}

auto joint::get_world() const -> world&
{
    return *_world;
}

distance_joint::distance_joint(b2Joint* b2joint, world* world)
    : joint {b2joint, world}
{
}

friction_joint::friction_joint(b2Joint* b2joint, world* world)
    : joint {b2joint, world}
{
}

gear_joint::gear_joint(b2Joint* b2joint, world* world)
    : joint {b2joint, world}
{
}

motor_joint::motor_joint(b2Joint* b2joint, world* world)
    : joint {b2joint, world}
{
}

mouse_joint::mouse_joint(b2Joint* b2joint, world* world)
    : joint {b2joint, world}
{
}

prismatic_joint::prismatic_joint(b2Joint* b2joint, world* world)
    : joint {b2joint, world}
{
}

pulley_joint::pulley_joint(b2Joint* b2joint, world* world)
    : joint {b2joint, world}
{
}

revolute_joint::revolute_joint(b2Joint* b2joint, world* world)
    : joint {b2joint, world}
{
}

weld_joint::weld_joint(b2Joint* b2joint, world* world)
    : joint {b2joint, world}
{
}

wheel_joint::wheel_joint(b2Joint* b2joint, world* world)
    : joint {b2joint, world}
{
}

void distance_joint_settings::initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB, point_f anchorA, point_f anchorB)
{
    b2DistanceJointDef def;
    def.Initialize(detail::get_impl(bodyA.get()), detail::get_impl(bodyB.get()), {anchorA.X, anchorA.Y}, {anchorB.X, anchorB.Y});
    BodyA = bodyA;
    BodyB = bodyB;

    LocalAnchorA = {def.localAnchorA.x, def.localAnchorA.y};
    LocalAnchorB = {def.localAnchorB.x, def.localAnchorB.y};
    Length       = def.length;
    MinLength    = def.minLength;
    MaxLength    = def.maxLength;
}

void friction_joint_settings::initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB, point_f anchor)
{
    b2FrictionJointDef def;
    def.Initialize(detail::get_impl(bodyA.get()), detail::get_impl(bodyB.get()), {anchor.X, anchor.Y});
    BodyA = bodyA;
    BodyB = bodyB;

    LocalAnchorA = {def.localAnchorA.x, def.localAnchorA.y};
    LocalAnchorB = {def.localAnchorB.x, def.localAnchorB.y};
}

void motor_joint_settings::initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB)
{
    b2MotorJointDef def;
    def.Initialize(detail::get_impl(bodyA.get()), detail::get_impl(bodyB.get()));
    BodyA = bodyA;
    BodyB = bodyB;

    LinearOffset  = {def.linearOffset.x, def.linearOffset.y};
    AngularOffset = def.angularOffset;
}

void prismatic_joint_settings::initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB, point_f anchor, point_f axis)
{
    b2PrismaticJointDef def;
    def.Initialize(detail::get_impl(bodyA.get()), detail::get_impl(bodyB.get()), {anchor.X, anchor.Y}, {axis.X, axis.Y});
    BodyA = bodyA;
    BodyB = bodyB;

    LocalAnchorA   = {def.localAnchorA.x, def.localAnchorB.y};
    LocalAnchorB   = {def.localAnchorB.x, def.localAnchorB.y};
    LocalAxisA     = {def.localAxisA.x, def.localAxisA.y};
    ReferenceAngle = def.referenceAngle;
}

pulley_joint_settings::pulley_joint_settings()
{
    IsCollideConnected = true;
}

void pulley_joint_settings::initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB,
                                       point_f groundAnchorA, point_f groundAnchorB,
                                       point_f anchorA, point_f anchorB,
                                       f32 ratio)
{
    b2PulleyJointDef def;
    def.Initialize(detail::get_impl(bodyA.get()), detail::get_impl(bodyB.get()),
                   {groundAnchorA.X, groundAnchorA.Y}, {groundAnchorB.X, groundAnchorB.Y},
                   {anchorA.X, anchorA.Y}, {anchorB.X, anchorB.Y}, ratio);
    BodyA = bodyA;
    BodyB = bodyB;

    GroundAnchorA = {def.groundAnchorA.x, def.groundAnchorA.y};
    GroundAnchorB = {def.groundAnchorB.x, def.groundAnchorB.y};
    LocalAnchorA  = {def.localAnchorA.x, def.localAnchorA.y};
    LocalAnchorB  = {def.localAnchorB.x, def.localAnchorB.y};
    LengthA       = def.lengthA;
    LengthB       = def.lengthB;
    Ratio         = def.ratio;
}

void revolute_joint_settings::initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB, point_f anchor)
{
    b2RevoluteJointDef def;
    def.Initialize(detail::get_impl(bodyA.get()), detail::get_impl(bodyB.get()), {anchor.X, anchor.Y});
    BodyA = bodyA;
    BodyB = bodyB;

    LocalAnchorA   = {def.localAnchorA.x, def.localAnchorA.y};
    LocalAnchorB   = {def.localAnchorB.x, def.localAnchorB.y};
    ReferenceAngle = radian_f {def.referenceAngle};
}

void weld_joint_settings::initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB, point_f anchor)
{
    b2WeldJointDef def;
    def.Initialize(detail::get_impl(bodyA.get()), detail::get_impl(bodyB.get()), {anchor.X, anchor.Y});
    BodyA = bodyA;
    BodyB = bodyB;

    LocalAnchorA   = {def.localAnchorA.x, def.localAnchorA.y};
    LocalAnchorB   = {def.localAnchorB.x, def.localAnchorB.y};
    ReferenceAngle = radian_f {def.referenceAngle};
}

void wheel_joint_settings::initialize(std::shared_ptr<body> const& bodyA, std::shared_ptr<body> const& bodyB, point_f anchor, point_f axis)
{
    b2WheelJointDef def;
    def.Initialize(detail::get_impl(bodyA.get()), detail::get_impl(bodyB.get()), {anchor.X, anchor.Y}, {axis.X, axis.Y});
    BodyA = bodyA;
    BodyB = bodyB;

    LocalAnchorA = {def.localAnchorA.x, def.localAnchorA.y};
    LocalAnchorB = {def.localAnchorB.x, def.localAnchorB.y};
    LocalAxisA   = {def.localAxisA.x, def.localAxisA.y};
}

} // namespace box2d

#endif
