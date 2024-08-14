// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "B2D.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

namespace tcob::physics::detail {
auto static to_b2Vec2(point_f val) -> b2Vec2
{
    return {val.X, val.Y};
}

b2dWorld::b2dWorld(point_f gravity)
{
    b2WorldDef worldDef {b2DefaultWorldDef()};
    worldDef.gravity = to_b2Vec2(gravity);
    ID               = b2CreateWorld(&worldDef);
}

b2dWorld::~b2dWorld()
{
    b2DestroyWorld(ID);
}

void b2dWorld::step(f32 delta, i32 subSteps) const
{
    b2World_Step(ID, delta, subSteps);
}

void b2dWorld::set_gravity(point_f value) const
{
    b2World_SetGravity(ID, to_b2Vec2(value));
}

void b2dWorld::set_allow_sleeping(bool value) const
{
    b2World_EnableSleeping(ID, value);
}

////////////////////////////////////////////////////////////

b2dBody::b2dBody(b2dWorld* world, body_transform const& xform, body_settings const& bodySettings)
{
    b2BodyDef def {b2DefaultBodyDef()};
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
    def.position        = to_b2Vec2(xform.Center);
    def.rotation        = b2MakeRot(xform.Angle.Value);
    def.linearVelocity  = to_b2Vec2(bodySettings.LinearVelocity);
    def.angularVelocity = bodySettings.AngularVelocity.Value;
    def.linearDamping   = bodySettings.LinearDamping;
    def.angularDamping  = bodySettings.AngularDamping;
    def.enableSleep     = bodySettings.EnableSleep;
    def.isAwake         = bodySettings.IsAwake;
    def.fixedRotation   = bodySettings.IsFixedRotation;
    def.isBullet        = bodySettings.IsBullet;
    def.isEnabled       = bodySettings.IsEnabled;
    def.gravityScale    = bodySettings.GravityScale;

    ID = b2CreateBody(world->ID, &def);
}

b2dBody::~b2dBody()
{
    b2DestroyBody(ID);
}

auto b2dBody::get_type() const -> body_type
{
    switch (b2Body_GetType(ID)) {
    case b2_staticBody:
        return body_type::Static;
    case b2_kinematicBody:
        return body_type::Kinematic;
    case b2_dynamicBody:
        return body_type::Dynamic;
    default: return {};
    }
}

void b2dBody::set_type(body_type type) const
{
    switch (type) {
    case body_type::Dynamic:
        b2Body_SetType(ID, b2_dynamicBody);
        break;
    case body_type::Static:
        b2Body_SetType(ID, b2_staticBody);
        break;
    case body_type::Kinematic:
        b2Body_SetType(ID, b2_kinematicBody);
        break;
    }
}

auto b2dBody::get_linear_velocity() const -> point_f
{
    auto const val {b2Body_GetLinearVelocity(ID)};
    return {val.x, val.y};
}

void b2dBody::set_linear_velocity(point_f value) const
{
    b2Body_SetLinearVelocity(ID, to_b2Vec2(value));
}

auto b2dBody::get_angular_velocity() const -> radian_f
{
    return b2Body_GetAngularVelocity(ID);
}

void b2dBody::set_angular_velocity(radian_f value) const
{
    b2Body_SetAngularVelocity(ID, value.Value);
}

auto b2dBody::get_linear_damping() const -> f32
{
    return b2Body_GetLinearDamping(ID);
}

void b2dBody::set_linear_damping(f32 value) const
{
    b2Body_SetLinearDamping(ID, value);
}

auto b2dBody::get_angular_damping() const -> f32
{
    return b2Body_GetAngularDamping(ID);
}

void b2dBody::set_angular_damping(f32 value) const
{
    b2Body_SetAngularDamping(ID, value);
}

auto b2dBody::get_allow_sleep() const -> bool
{
    return b2Body_IsSleepEnabled(ID);
}

void b2dBody::set_allow_sleep(bool value) const
{
    b2Body_EnableSleep(ID, value);
}

auto b2dBody::get_awake() const -> bool
{
    return b2Body_IsAwake(ID);
}

void b2dBody::set_awake(bool value) const
{
    b2Body_SetAwake(ID, value);
}

auto b2dBody::get_fixed_rotation() const -> bool
{
    return b2Body_IsFixedRotation(ID);
}

void b2dBody::set_fixed_rotation(bool value) const
{
    b2Body_SetFixedRotation(ID, value);
}

auto b2dBody::get_bullet() const -> bool
{
    return b2Body_IsBullet(ID);
}

void b2dBody::set_bullet(bool value) const
{
    b2Body_SetBullet(ID, value);
}

auto b2dBody::get_enabled() const -> bool
{
    return b2Body_IsEnabled(ID);
}

void b2dBody::set_enabled(bool value) const
{
    if (!value) {
        b2Body_Disable(ID);
    } else {
        b2Body_Enable(ID);
    }
}

auto b2dBody::get_gravity_scale() const -> f32
{
    return b2Body_GetGravityScale(ID);
}

void b2dBody::set_gravity_scale(f32 value) const
{
    b2Body_SetGravityScale(ID, value);
}

auto b2dBody::get_transform() const -> body_transform
{
    auto val {b2Body_GetTransform(ID)};
    return {{val.p.x, val.p.y}, b2Rot_GetAngle(val.q)};
}

void b2dBody::set_transform(body_transform value) const
{
    b2Body_SetTransform(ID, to_b2Vec2(value.Center), b2MakeRot(value.Angle.Value));
}

auto b2dBody::get_center() const -> point_f
{
    auto val {b2Body_GetWorldCenterOfMass(ID)};
    return {val.x, val.y};
}

auto b2dBody::get_local_center() const -> point_f
{
    auto val {b2Body_GetLocalCenterOfMass(ID)};
    return {val.x, val.y};
}

void b2dBody::apply_force(point_f force, point_f point, bool wake) const
{
    b2Body_ApplyForce(ID, to_b2Vec2(force), to_b2Vec2(point), wake);
}

void b2dBody::apply_force_to_center(point_f force, bool wake) const
{
    b2Body_ApplyForceToCenter(ID, to_b2Vec2(force), wake);
}

void b2dBody::apply_linear_impulse(point_f imp, point_f point, bool wake) const
{
    b2Body_ApplyLinearImpulse(ID, to_b2Vec2(imp), to_b2Vec2(point), wake);
}

void b2dBody::apply_linear_impulse_to_center(point_f imp, bool wake) const
{
    b2Body_ApplyLinearImpulseToCenter(ID, to_b2Vec2(imp), wake);
}

void b2dBody::apply_torque(f32 torque, bool wake) const
{
    b2Body_ApplyTorque(ID, torque, wake);
}

void b2dBody::apply_angular_impulse(f32 impulse, bool wake) const
{
    b2Body_ApplyAngularImpulse(ID, impulse, wake);
}

////////////////////////////////////////////////////////////

b2dJoint::b2dJoint(b2dWorld* world, distance_joint_settings const& jointSettings)
{
    auto def {b2DefaultDistanceJointDef()};
    def.bodyIdA          = jointSettings.BodyA->_impl->ID;
    def.bodyIdB          = jointSettings.BodyB->_impl->ID;
    def.collideConnected = jointSettings.IsCollideConnected;
    def.localAnchorA     = to_b2Vec2(jointSettings.LocalAnchorA);
    def.localAnchorB     = to_b2Vec2(jointSettings.LocalAnchorB);
    def.length           = jointSettings.Length;
    def.enableSpring     = jointSettings.EnableSpring;
    def.hertz            = jointSettings.Hertz;
    def.dampingRatio     = jointSettings.DampingRatio;
    def.enableLimit      = jointSettings.EnableLimit;
    def.minLength        = jointSettings.MinLength;
    def.maxLength        = jointSettings.MaxLength;
    def.enableMotor      = jointSettings.EnableMotor;
    def.maxMotorForce    = jointSettings.MaxMotorForce;
    def.motorSpeed       = jointSettings.MotorSpeed;

    ID = b2CreateDistanceJoint(world->ID, &def);
}

b2dJoint::b2dJoint(b2dWorld* world, motor_joint_settings const& jointSettings)
{
    auto def {b2DefaultMotorJointDef()};
    def.bodyIdA          = jointSettings.BodyA->_impl->ID;
    def.bodyIdB          = jointSettings.BodyB->_impl->ID;
    def.collideConnected = jointSettings.IsCollideConnected;
    def.linearOffset     = to_b2Vec2(jointSettings.LinearOffset);
    def.angularOffset    = jointSettings.AngularOffset.Value;
    def.maxForce         = jointSettings.MaxForce;
    def.maxTorque        = jointSettings.MaxTorque;
    def.correctionFactor = jointSettings.CorrectionFactor;

    ID = b2CreateMotorJoint(world->ID, &def);
}

b2dJoint::b2dJoint(b2dWorld* world, mouse_joint_settings const& jointSettings)
{
    auto def {b2DefaultMouseJointDef()};
    def.bodyIdA          = jointSettings.BodyA->_impl->ID;
    def.bodyIdB          = jointSettings.BodyB->_impl->ID;
    def.collideConnected = jointSettings.IsCollideConnected;
    def.hertz            = jointSettings.Hertz;
    def.dampingRatio     = jointSettings.DampingRatio;
    def.maxForce         = jointSettings.MaxForce;

    ID = b2CreateMouseJoint(world->ID, &def);
}

b2dJoint::b2dJoint(b2dWorld* world, prismatic_joint_settings const& jointSettings)
{
    auto def {b2DefaultPrismaticJointDef()};
    def.bodyIdA          = jointSettings.BodyA->_impl->ID;
    def.bodyIdB          = jointSettings.BodyB->_impl->ID;
    def.collideConnected = jointSettings.IsCollideConnected;
    def.localAnchorA     = to_b2Vec2(jointSettings.LocalAnchorA);
    def.localAnchorB     = to_b2Vec2(jointSettings.LocalAnchorB);
    def.localAxisA       = to_b2Vec2(jointSettings.LocalAxisA);
    def.enableSpring     = jointSettings.EnableSpring;
    def.hertz            = jointSettings.Hertz;
    def.dampingRatio     = jointSettings.DampingRatio;
    def.enableLimit      = jointSettings.EnableLimit;
    def.lowerTranslation = jointSettings.LowerTranslation;
    def.upperTranslation = jointSettings.UpperTranslation;
    def.enableMotor      = jointSettings.EnableMotor;
    def.maxMotorForce    = jointSettings.MaxMotorForce;
    def.motorSpeed       = jointSettings.MotorSpeed;

    ID = b2CreatePrismaticJoint(world->ID, &def);
}

b2dJoint::b2dJoint(b2dWorld* world, revolute_joint_settings const& jointSettings)
{
    auto def {b2DefaultRevoluteJointDef()};
    def.bodyIdA          = jointSettings.BodyA->_impl->ID;
    def.bodyIdB          = jointSettings.BodyB->_impl->ID;
    def.collideConnected = jointSettings.IsCollideConnected;
    def.localAnchorA     = to_b2Vec2(jointSettings.LocalAnchorA);
    def.localAnchorB     = to_b2Vec2(jointSettings.LocalAnchorB);
    def.referenceAngle   = jointSettings.ReferenceAngle.Value;
    def.enableSpring     = jointSettings.EnableSpring;
    def.hertz            = jointSettings.Hertz;
    def.dampingRatio     = jointSettings.DampingRatio;
    def.enableLimit      = jointSettings.EnableLimit;
    def.lowerAngle       = jointSettings.LowerAngle;
    def.upperAngle       = jointSettings.UpperAngle;
    def.enableMotor      = jointSettings.EnableMotor;
    def.maxMotorTorque   = jointSettings.MaxMotorTorque;
    def.motorSpeed       = jointSettings.MotorSpeed;
    def.drawSize         = jointSettings.DrawSize;

    ID = b2CreateRevoluteJoint(world->ID, &def);
}

b2dJoint::b2dJoint(b2dWorld* world, weld_joint_settings const& jointSettings)
{
    auto def {b2DefaultWeldJointDef()};
    def.bodyIdA             = jointSettings.BodyA->_impl->ID;
    def.bodyIdB             = jointSettings.BodyB->_impl->ID;
    def.collideConnected    = jointSettings.IsCollideConnected;
    def.referenceAngle      = jointSettings.ReferenceAngle.Value;
    def.linearHertz         = jointSettings.LinearHertz;
    def.angularHertz        = jointSettings.AngularHertz;
    def.linearDampingRatio  = jointSettings.LinearDampingRatio;
    def.angularDampingRatio = jointSettings.AngularDampingRatio;

    ID = b2CreateWeldJoint(world->ID, &def);
}

b2dJoint::b2dJoint(b2dWorld* world, wheel_joint_settings const& jointSettings)
{
    auto def {b2DefaultWheelJointDef()};
    def.bodyIdA          = jointSettings.BodyA->_impl->ID;
    def.bodyIdB          = jointSettings.BodyB->_impl->ID;
    def.collideConnected = jointSettings.IsCollideConnected;
    def.enableSpring     = jointSettings.EnableSpring;
    def.hertz            = jointSettings.Hertz;
    def.dampingRatio     = jointSettings.DampingRatio;
    def.enableLimit      = jointSettings.EnableLimit;
    def.lowerTranslation = jointSettings.LowerTranslation;
    def.upperTranslation = jointSettings.UpperTranslation;
    def.enableMotor      = jointSettings.EnableMotor;
    def.maxMotorTorque   = jointSettings.MaxMotorTorque;
    def.motorSpeed       = jointSettings.MotorSpeed;

    ID = b2CreateWheelJoint(world->ID, &def);
}

b2dJoint::~b2dJoint()
{
    b2DestroyJoint(ID);
}

////////////////////////////////////////////////////////////

b2dShape::b2dShape(b2dBody* body, polygon_shape_settings const& shapeSettings)
{
    std::vector<b2Vec2> v;
    v.reserve(shapeSettings.Verts.size());
    for (auto const& vert : shapeSettings.Verts) {
        v.emplace_back(vert.X, vert.Y);
    }

    b2Hull h {b2ComputeHull(v.data(), static_cast<i32>(v.size()))};
    auto   poly {b2MakePolygon(&h, shapeSettings.Radius)};

    b2ShapeDef shapeDef {b2DefaultShapeDef()};
    shapeDef.friction             = shapeSettings.Friction;
    shapeDef.restitution          = shapeSettings.Restitution;
    shapeDef.density              = shapeSettings.Density;
    shapeDef.isSensor             = shapeSettings.IsSensor;
    shapeDef.enableSensorEvents   = shapeSettings.EnableSensorEvents;
    shapeDef.enableContactEvents  = shapeSettings.EnableContactEvents;
    shapeDef.enableHitEvents      = shapeSettings.EnableHitEvents;
    shapeDef.enablePreSolveEvents = shapeSettings.EnablePreSolveEvents;

    ID = b2CreatePolygonShape(body->ID, &shapeDef, &poly);
}

b2dShape::b2dShape(b2dBody* body, rect_shape_settings const& shapeSettings)
{
    auto const& rect {shapeSettings.Extents};
    auto        poly {b2MakeOffsetBox(rect.Width / 2, rect.Height / 2, to_b2Vec2(rect.top_left()), shapeSettings.Angle.Value)};

    b2ShapeDef shapeDef {b2DefaultShapeDef()};
    shapeDef.friction             = shapeSettings.Friction;
    shapeDef.restitution          = shapeSettings.Restitution;
    shapeDef.density              = shapeSettings.Density;
    shapeDef.isSensor             = shapeSettings.IsSensor;
    shapeDef.enableSensorEvents   = shapeSettings.EnableSensorEvents;
    shapeDef.enableContactEvents  = shapeSettings.EnableContactEvents;
    shapeDef.enableHitEvents      = shapeSettings.EnableHitEvents;
    shapeDef.enablePreSolveEvents = shapeSettings.EnablePreSolveEvents;

    ID = b2CreatePolygonShape(body->ID, &shapeDef, &poly);
}

b2dShape::b2dShape(b2dBody* body, circle_shape_settings const& shapeSettings)
{
    b2Circle poly {to_b2Vec2(shapeSettings.Center), shapeSettings.Radius};

    b2ShapeDef shapeDef {b2DefaultShapeDef()};
    shapeDef.friction             = shapeSettings.Friction;
    shapeDef.restitution          = shapeSettings.Restitution;
    shapeDef.density              = shapeSettings.Density;
    shapeDef.isSensor             = shapeSettings.IsSensor;
    shapeDef.enableSensorEvents   = shapeSettings.EnableSensorEvents;
    shapeDef.enableContactEvents  = shapeSettings.EnableContactEvents;
    shapeDef.enableHitEvents      = shapeSettings.EnableHitEvents;
    shapeDef.enablePreSolveEvents = shapeSettings.EnablePreSolveEvents;

    ID = b2CreateCircleShape(body->ID, &shapeDef, &poly);
}

b2dShape::b2dShape(b2dBody* body, segment_shape_settings const& shapeSettings)
{
    b2Segment poly {to_b2Vec2(shapeSettings.Point0), to_b2Vec2(shapeSettings.Point1)};

    b2ShapeDef shapeDef {b2DefaultShapeDef()};
    shapeDef.friction             = shapeSettings.Friction;
    shapeDef.restitution          = shapeSettings.Restitution;
    shapeDef.density              = shapeSettings.Density;
    shapeDef.isSensor             = shapeSettings.IsSensor;
    shapeDef.enableSensorEvents   = shapeSettings.EnableSensorEvents;
    shapeDef.enableContactEvents  = shapeSettings.EnableContactEvents;
    shapeDef.enableHitEvents      = shapeSettings.EnableHitEvents;
    shapeDef.enablePreSolveEvents = shapeSettings.EnablePreSolveEvents;

    ID = b2CreateSegmentShape(body->ID, &shapeDef, &poly);
}

b2dShape::~b2dShape()
{
    b2DestroyShape(ID);
}

}

#endif
