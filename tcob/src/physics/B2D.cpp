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

b2d_world::b2d_world(point_f gravity)
{
    b2WorldDef worldDef {b2DefaultWorldDef()};
    worldDef.gravity = to_b2Vec2(gravity);
    ID               = b2CreateWorld(&worldDef);
}

b2d_world::~b2d_world()
{
    b2DestroyWorld(ID);
}

void b2d_world::step(f32 delta, i32 subSteps) const
{
    b2World_Step(ID, delta, subSteps);
}

void b2d_world::set_gravity(point_f value) const
{
    b2World_SetGravity(ID, to_b2Vec2(value));
}

void b2d_world::set_enable_sleeping(bool value) const
{
    b2World_EnableSleeping(ID, value);
}

void b2d_world::draw(b2d_debug_draw* draw, debug_draw::settings const& settings) const
{
    draw->apply_settings(settings);
    b2World_Draw(ID, &draw->ID);
}

////////////////////////////////////////////////////////////

b2d_body::b2d_body(b2d_world* world, body_transform const& xform, body_settings const& bodySettings)
{
    b2BodyDef def {b2DefaultBodyDef()};
    switch (bodySettings.Type) {
    case body_type::Dynamic: def.type = b2_dynamicBody; break;
    case body_type::Static: def.type = b2_staticBody; break;
    case body_type::Kinematic: def.type = b2_kinematicBody; break;
    }
    def.position          = to_b2Vec2(xform.Center);
    def.rotation          = b2MakeRot(xform.Angle.Value);
    def.linearVelocity    = to_b2Vec2(bodySettings.LinearVelocity);
    def.angularVelocity   = bodySettings.AngularVelocity.Value;
    def.linearDamping     = bodySettings.LinearDamping;
    def.angularDamping    = bodySettings.AngularDamping;
    def.enableSleep       = bodySettings.EnableSleep;
    def.isAwake           = bodySettings.IsAwake;
    def.fixedRotation     = bodySettings.IsFixedRotation;
    def.isBullet          = bodySettings.IsBullet;
    def.isEnabled         = bodySettings.IsEnabled;
    def.gravityScale      = bodySettings.GravityScale;
    def.sleepThreshold    = bodySettings.SleepThreshold;
    def.automaticMass     = bodySettings.AutomaticMass;
    def.allowFastRotation = bodySettings.AllowFastRotation;

    ID = b2CreateBody(world->ID, &def);
}

b2d_body::~b2d_body()
{
    b2DestroyBody(ID);
}

auto b2d_body::get_type() const -> body_type
{
    switch (b2Body_GetType(ID)) {
    case b2_staticBody: return body_type::Static;
    case b2_kinematicBody: return body_type::Kinematic;
    case b2_dynamicBody: return body_type::Dynamic;
    default: return {};
    }
}

void b2d_body::set_type(body_type type) const
{
    switch (type) {
    case body_type::Dynamic: b2Body_SetType(ID, b2_dynamicBody); break;
    case body_type::Static: b2Body_SetType(ID, b2_staticBody); break;
    case body_type::Kinematic: b2Body_SetType(ID, b2_kinematicBody); break;
    }
}

auto b2d_body::get_linear_velocity() const -> point_f
{
    auto const val {b2Body_GetLinearVelocity(ID)};
    return {val.x, val.y};
}

void b2d_body::set_linear_velocity(point_f value) const
{
    b2Body_SetLinearVelocity(ID, to_b2Vec2(value));
}

auto b2d_body::get_angular_velocity() const -> radian_f
{
    return b2Body_GetAngularVelocity(ID);
}

void b2d_body::set_angular_velocity(radian_f value) const
{
    b2Body_SetAngularVelocity(ID, value.Value);
}

auto b2d_body::get_linear_damping() const -> f32
{
    return b2Body_GetLinearDamping(ID);
}

void b2d_body::set_linear_damping(f32 value) const
{
    b2Body_SetLinearDamping(ID, value);
}

auto b2d_body::get_angular_damping() const -> f32
{
    return b2Body_GetAngularDamping(ID);
}

void b2d_body::set_angular_damping(f32 value) const
{
    b2Body_SetAngularDamping(ID, value);
}

auto b2d_body::get_enable_sleep() const -> bool
{
    return b2Body_IsSleepEnabled(ID);
}

void b2d_body::set_enable_sleep(bool value) const
{
    b2Body_EnableSleep(ID, value);
}

auto b2d_body::get_awake() const -> bool
{
    return b2Body_IsAwake(ID);
}

void b2d_body::set_awake(bool value) const
{
    b2Body_SetAwake(ID, value);
}

auto b2d_body::get_fixed_rotation() const -> bool
{
    return b2Body_IsFixedRotation(ID);
}

void b2d_body::set_fixed_rotation(bool value) const
{
    b2Body_SetFixedRotation(ID, value);
}

auto b2d_body::get_bullet() const -> bool
{
    return b2Body_IsBullet(ID);
}

void b2d_body::set_bullet(bool value) const
{
    b2Body_SetBullet(ID, value);
}

auto b2d_body::get_enabled() const -> bool
{
    return b2Body_IsEnabled(ID);
}

void b2d_body::set_enabled(bool value) const
{
    if (!value) {
        b2Body_Disable(ID);
    } else {
        b2Body_Enable(ID);
    }
}

auto b2d_body::get_gravity_scale() const -> f32
{
    return b2Body_GetGravityScale(ID);
}

void b2d_body::set_gravity_scale(f32 value) const
{
    b2Body_SetGravityScale(ID, value);
}

auto b2d_body::get_transform() const -> body_transform
{
    auto val {b2Body_GetTransform(ID)};
    return {{val.p.x, val.p.y}, b2Rot_GetAngle(val.q)};
}

void b2d_body::set_transform(body_transform value) const
{
    b2Body_SetTransform(ID, to_b2Vec2(value.Center), b2MakeRot(value.Angle.Value));
}

auto b2d_body::get_center() const -> point_f
{
    auto val {b2Body_GetWorldCenterOfMass(ID)};
    return {val.x, val.y};
}

auto b2d_body::get_local_center() const -> point_f
{
    auto val {b2Body_GetLocalCenterOfMass(ID)};
    return {val.x, val.y};
}

void b2d_body::apply_force(point_f force, point_f point, bool wake) const
{
    b2Body_ApplyForce(ID, to_b2Vec2(force), to_b2Vec2(point), wake);
}

void b2d_body::apply_force_to_center(point_f force, bool wake) const
{
    b2Body_ApplyForceToCenter(ID, to_b2Vec2(force), wake);
}

void b2d_body::apply_linear_impulse(point_f imp, point_f point, bool wake) const
{
    b2Body_ApplyLinearImpulse(ID, to_b2Vec2(imp), to_b2Vec2(point), wake);
}

void b2d_body::apply_linear_impulse_to_center(point_f imp, bool wake) const
{
    b2Body_ApplyLinearImpulseToCenter(ID, to_b2Vec2(imp), wake);
}

void b2d_body::apply_torque(f32 torque, bool wake) const
{
    b2Body_ApplyTorque(ID, torque, wake);
}

void b2d_body::apply_angular_impulse(f32 impulse, bool wake) const
{
    b2Body_ApplyAngularImpulse(ID, impulse, wake);
}

////////////////////////////////////////////////////////////

b2d_joint::b2d_joint(b2d_world* world, distance_joint_settings const& jointSettings)
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

b2d_joint::b2d_joint(b2d_world* world, motor_joint_settings const& jointSettings)
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

b2d_joint::b2d_joint(b2d_world* world, mouse_joint_settings const& jointSettings)
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

b2d_joint::b2d_joint(b2d_world* world, prismatic_joint_settings const& jointSettings)
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

b2d_joint::b2d_joint(b2d_world* world, revolute_joint_settings const& jointSettings)
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

b2d_joint::b2d_joint(b2d_world* world, weld_joint_settings const& jointSettings)
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

b2d_joint::b2d_joint(b2d_world* world, wheel_joint_settings const& jointSettings)
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

b2d_joint::~b2d_joint()
{
    b2DestroyJoint(ID);
}

////////////////////////////////////////////////////////////

b2d_shape::b2d_shape(b2d_body* body, polygon_shape_settings const& shapeSettings)
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

b2d_shape::b2d_shape(b2d_body* body, rect_shape_settings const& shapeSettings)
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

b2d_shape::b2d_shape(b2d_body* body, circle_shape_settings const& shapeSettings)
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

b2d_shape::b2d_shape(b2d_body* body, segment_shape_settings const& shapeSettings)
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

b2d_shape::~b2d_shape()
{
    b2DestroyShape(ID);
}

////////////////////////////////////////////////////////////

extern "C" {
void static DrawPolygon(b2Vec2 const* vertices, int vertexCount, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};

    std::span<b2Vec2 const> span {vertices, static_cast<usize>(vertexCount)};
    std::vector<point_f>    verts;
    verts.reserve(vertexCount);
    for (auto const& v : span) { verts.emplace_back(v.x, v.y); }

    ddraw->draw_polygon(verts, color::FromRGB(color));
}

void static DrawSolidPolygon(b2Transform transform, b2Vec2 const* vertices, int vertexCount, float radius, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};

    std::span<b2Vec2 const> span {vertices, static_cast<usize>(vertexCount)};
    std::vector<point_f>    verts;
    verts.reserve(vertexCount);
    for (auto const& v : span) { verts.emplace_back(v.x, v.y); }

    ddraw->draw_solid_polygon({{transform.p.x, transform.p.y}, b2Rot_GetAngle(transform.q)}, verts, radius, color::FromRGB(color));
}

void static DrawCircle(b2Vec2 center, float radius, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->draw_circle({center.x, center.y}, radius, color::FromRGB(color));
}

void static DrawSolidCircle(b2Transform transform, float radius, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->draw_solid_circle({{transform.p.x, transform.p.y}, b2Rot_GetAngle(transform.q)}, radius, color::FromRGB(color));
}

void static DrawCapsule(b2Vec2 /* p1 */, b2Vec2 /* p2 */, float /* radius */, b2HexColor /* color */, void* /* context */)
{
    // auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
}

void static DrawSolidCapsule(b2Vec2 /* p1 */, b2Vec2 /* p2 */, float /* radius */, b2HexColor /* color */, void* /* context */)
{
    //  auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
}

void static DrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->draw_segment({p1.x, p1.y}, {p2.x, p2.y}, color::FromRGB(color));
}

void static DrawTransform(b2Transform transform, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->draw_transform({{transform.p.x, transform.p.y}, b2Rot_GetAngle(transform.q)});
}

void static DrawPoint(b2Vec2 p, float size, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->draw_point({p.x, p.y}, size, color::FromRGB(color));
}

void static DrawString(b2Vec2 p, char const* s, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->draw_string({p.x, p.y}, s);
}
}

b2d_debug_draw::b2d_debug_draw(debug_draw* parent)
    : _parent {parent}
{
    ID.context          = this;
    ID.DrawPolygon      = &DrawPolygon;
    ID.DrawSolidPolygon = &DrawSolidPolygon;
    ID.DrawCircle       = &DrawCircle;
    ID.DrawSolidCircle  = &DrawSolidCircle;
    ID.DrawCapsule      = &DrawCapsule;      // TODO
    ID.DrawSolidCapsule = &DrawSolidCapsule; // TODO
    ID.DrawSegment      = &DrawSegment;
    ID.DrawTransform    = &DrawTransform;
    ID.DrawPoint        = &DrawPoint;
    ID.DrawString       = &DrawString; // TODO
}

void b2d_debug_draw::draw_polygon(std::span<point_f const> vertices, color color)
{
    _parent->draw_polygon(vertices, color);
}

void b2d_debug_draw::draw_solid_polygon(body_transform xform, std::span<point_f const> vertices, f32 radius, color color)
{
    _parent->draw_solid_polygon(xform, vertices, radius, color);
}

void b2d_debug_draw::draw_circle(point_f center, f32 radius, color color)
{
    _parent->draw_circle(center, radius, color);
}

void b2d_debug_draw::draw_solid_circle(body_transform xform, f32 radius, color color)
{
    _parent->draw_solid_circle(xform, radius, color);
}

void b2d_debug_draw::draw_segment(point_f p1, point_f p2, color color)
{
    _parent->draw_segment(p1, p2, color);
}

void b2d_debug_draw::draw_transform(body_transform const& xf)
{
    _parent->draw_transform(xf);
}

void b2d_debug_draw::draw_point(point_f p, f32 size, color color)
{
    _parent->draw_point(p, size, color);
}

void b2d_debug_draw::draw_string(point_f p, string const& text)
{
    _parent->draw_string(p, text);
}

void b2d_debug_draw::apply_settings(debug_draw::settings const& settings)
{
    if (settings.DrawingBounds) {
        ID.drawingBounds    = {to_b2Vec2(settings.DrawingBounds->LowerBounds), to_b2Vec2(settings.DrawingBounds->UpperBounds)};
        ID.useDrawingBounds = true;
    }

    ID.drawShapes           = settings.DrawShapes;
    ID.drawJoints           = settings.DrawJoints;
    ID.drawJointExtras      = settings.DrawJointExtras;
    ID.drawAABBs            = settings.DrawAABBs;
    ID.drawContacts         = settings.DrawContacts;
    ID.drawGraphColors      = settings.DrawGraphColors;
    ID.drawContactNormals   = settings.DrawContactNormals;
    ID.drawContactImpulses  = settings.DrawContactImpulses;
    ID.drawFrictionImpulses = settings.DrawFrictionImpulses;
}
}

#endif
