// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "B2D.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <cstdint>
    #include <span>
    #include <vector>

    #include <box2d/base.h>
    #include <box2d/box2d.h>
    #include <box2d/collision.h>
    #include <box2d/id.h>
    #include <box2d/math_functions.h>
    #include <box2d/types.h>

    #include "tcob/core/AngleUnits.hpp"
    #include "tcob/core/Color.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/core/Rect.hpp"
    #include "tcob/core/ServiceLocator.hpp"
    #include "tcob/core/StringUtils.hpp"
    #include "tcob/core/TaskManager.hpp"
    #include "tcob/physics/Body.hpp"
    #include "tcob/physics/DebugDraw.hpp"
    #include "tcob/physics/Joint.hpp"
    #include "tcob/physics/Physics.hpp"
    #include "tcob/physics/Shape.hpp"
    #include "tcob/physics/World.hpp"

namespace tcob::physics::detail {
auto static to_b2Vec2(point_f val) -> b2Vec2
{
    return {val.X, val.Y};
}

auto static to_point_f(b2Vec2 val) -> point_f
{
    return {val.x, val.y};
}

auto static get_body(b2BodyId id) -> body*
{
    return reinterpret_cast<body*>(b2Body_GetUserData(id));
}

auto static get_shape(b2ShapeId id) -> shape*
{
    return reinterpret_cast<shape*>(b2Shape_GetUserData(id));
}

b2d_world::b2d_world(world::settings const& settings)
{
    b2WorldDef worldDef {b2DefaultWorldDef()};
    worldDef.gravity              = to_b2Vec2(settings.Gravity);
    worldDef.restitutionThreshold = settings.RestitutionThreshold;
    worldDef.hitEventThreshold    = settings.HitEventThreshold;
    worldDef.contactHertz         = settings.ContactHertz;
    worldDef.contactDampingRatio  = settings.ContactDampingRatio;
    worldDef.maxContactPushSpeed  = settings.MaxContactPushSpeed;
    worldDef.jointHertz           = settings.JointHertz;
    worldDef.jointDampingRatio    = settings.JointDampingRatio;
    worldDef.maximumLinearSpeed   = settings.MaximumLinearSpeed;
    worldDef.enableSleep          = settings.EnableSleeping;
    worldDef.enableContinuous     = settings.EnableContinuous;

    worldDef.workerCount = static_cast<i32>(locate_service<task_manager>().thread_count());
    worldDef.enqueueTask = [](b2TaskCallback* task, int32_t itemCount, int32_t minRange, void* taskContext, void* /* userContext */) -> void* {
        locate_service<task_manager>().run_parallel(
            [task, taskContext](par_task const& ctx) {
                task(static_cast<i32>(ctx.Start), static_cast<i32>(ctx.End), static_cast<i32>(ctx.Thread), taskContext);
            },
            itemCount, minRange);

        return nullptr;
    };
    worldDef.finishTask = [](void* /* userTask */, void* /* userContext */) {

    };
    ID = b2CreateWorld(&worldDef);
}

b2d_world::~b2d_world()
{
    b2DestroyWorld(ID);
}

void b2d_world::step(f32 delta, i32 subSteps) const
{
    b2World_Step(ID, delta, subSteps);
}

auto b2d_world::get_gravity() const -> point_f
{
    return to_point_f(b2World_GetGravity(ID));
}

void b2d_world::set_gravity(point_f value) const
{
    b2World_SetGravity(ID, to_b2Vec2(value));
}

auto b2d_world::get_enable_sleeping() const -> bool
{
    return b2World_IsSleepingEnabled(ID);
}

void b2d_world::set_enable_sleeping(bool value) const
{
    b2World_EnableSleeping(ID, value);
}

auto b2d_world::get_enable_continuous() const -> bool
{
    return b2World_IsContinuousEnabled(ID);
}

void b2d_world::set_enable_continuous(bool value) const
{
    b2World_EnableContinuous(ID, value);
}

auto b2d_world::get_restitution_threshold() const -> f32
{
    return b2World_GetRestitutionThreshold(ID);
}

void b2d_world::set_restitution_threshold(f32 value) const
{
    b2World_SetRestitutionThreshold(ID, value);
}

auto b2d_world::get_hit_event_threshold() const -> f32
{
    return b2World_GetHitEventThreshold(ID);
}

void b2d_world::set_hit_event_threshold(f32 value) const
{
    b2World_SetHitEventThreshold(ID, value);
}

auto b2d_world::get_maximum_linear_speed() const -> f32
{
    return b2World_GetMaximumLinearSpeed(ID);
}

void b2d_world::set_maximum_linear_speed(f32 value) const
{
    b2World_SetMaximumLinearSpeed(ID, value);
}

void b2d_world::explode(explosion const& explosion) const
{
    b2ExplosionDef def;
    def.falloff          = explosion.Falloff;
    def.impulsePerLength = explosion.ImpulsePerLength;
    def.maskBits         = explosion.MaskBits;
    def.position         = to_b2Vec2(explosion.Position);
    def.radius           = explosion.Radius;
    b2World_Explode(ID, &def);
}

void b2d_world::draw(b2d_debug_draw* draw, debug_draw::settings const& settings) const
{
    draw->apply_settings(settings);
    b2World_Draw(ID, &draw->ID);
}

auto b2d_world::get_body_events() const -> body_events
{
    auto const  events {b2World_GetBodyEvents(ID)};
    body_events retValue;
    retValue.Move.reserve(events.moveCount);

    std::span<b2BodyMoveEvent> const move {events.moveEvents, static_cast<usize>(events.moveCount)};
    for (auto& event : move) {
        body_move_event ev;
        ev.Body       = get_body(event.bodyId);
        ev.Transform  = {{event.transform.p.x, event.transform.p.y}, radian_f {b2Rot_GetAngle(event.transform.q)}};
        ev.FellAsleep = event.fellAsleep;
        retValue.Move.push_back(ev);
    }

    return retValue;
}

auto b2d_world::get_contact_events() const -> contact_events
{
    auto const     events {b2World_GetContactEvents(ID)};
    contact_events retValue;
    retValue.BeginTouch.reserve(events.beginCount);
    retValue.EndTouch.reserve(events.endCount);
    retValue.Hit.reserve(events.hitCount);

    std::span<b2ContactBeginTouchEvent> const begin {events.beginEvents, static_cast<usize>(events.beginCount)};
    for (auto& event : begin) {
        contact_begin_touch_event ev;
        ev.ShapeA = get_shape(event.shapeIdA);
        ev.ShapeB = get_shape(event.shapeIdB);
        retValue.BeginTouch.push_back(ev);
    }
    std::span<b2ContactEndTouchEvent> const end {events.endEvents, static_cast<usize>(events.endCount)};
    for (auto& event : end) {
        contact_end_touch_event ev;
        ev.ShapeA = get_shape(event.shapeIdA);
        ev.ShapeB = get_shape(event.shapeIdB);
        retValue.EndTouch.push_back(ev);
    }

    std::span<b2ContactHitEvent> const hit {events.hitEvents, static_cast<usize>(events.hitCount)};
    for (auto& event : hit) {
        contact_hit_event ev;
        ev.ShapeA        = get_shape(event.shapeIdA);
        ev.ShapeB        = get_shape(event.shapeIdB);
        ev.ApproachSpeed = event.approachSpeed;
        ev.Normal        = {event.normal.x, event.normal.y};
        ev.Point         = {event.point.x, event.point.y};
        retValue.Hit.push_back(ev);
    }

    return retValue;
}

auto b2d_world::get_sensor_events() const -> sensor_events
{
    auto const    events {b2World_GetSensorEvents(ID)};
    sensor_events retValue;
    retValue.BeginTouch.reserve(static_cast<usize>(events.beginCount));
    retValue.EndTouch.reserve(static_cast<usize>(events.endCount));

    std::span<b2SensorBeginTouchEvent> const begin {events.beginEvents, static_cast<usize>(events.beginCount)};
    for (auto& event : begin) {
        retValue.BeginTouch.emplace_back(get_shape(event.sensorShapeId), get_shape(event.visitorShapeId));
    }
    std::span<b2SensorEndTouchEvent> const end {events.endEvents, static_cast<usize>(events.endCount)};
    for (auto& event : end) {
        retValue.EndTouch.emplace_back(get_shape(event.sensorShapeId), get_shape(event.visitorShapeId));
    }
    return retValue;
}

void b2d_world::set_joint_tuning(f32 hertz, f32 damping) const
{
    b2World_SetJointTuning(ID, hertz, damping);
}

void b2d_world::set_contact_tuning(f32 hertz, f32 damping, f32 pushSpeed) const
{
    b2World_SetContactTuning(ID, hertz, damping, pushSpeed);
}

auto b2d_world::get_awake_body_count() const -> i32
{
    return b2World_GetAwakeBodyCount(ID);
}

auto b2d_world::get_enable_warm_starting() const -> bool
{
    return b2World_IsWarmStartingEnabled(ID);
}

void b2d_world::set_enable_warm_starting(bool value) const
{
    b2World_EnableWarmStarting(ID, value);
}

////////////////////////////////////////////////////////////

b2d_body::b2d_body(b2d_world* world, body_transform const& xform, body::settings const& bodySettings)
{
    b2BodyDef def {b2DefaultBodyDef()};
    switch (bodySettings.Type) {
    case body_type::Dynamic:   def.type = b2_dynamicBody; break;
    case body_type::Static:    def.type = b2_staticBody; break;
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
    case b2_staticBody:    return body_type::Static;
    case b2_kinematicBody: return body_type::Kinematic;
    case b2_dynamicBody:   return body_type::Dynamic;
    default:               return {};
    }
}

void b2d_body::set_type(body_type type) const
{
    switch (type) {
    case body_type::Dynamic:   b2Body_SetType(ID, b2_dynamicBody); break;
    case body_type::Static:    b2Body_SetType(ID, b2_staticBody); break;
    case body_type::Kinematic: b2Body_SetType(ID, b2_kinematicBody); break;
    }
}

auto b2d_body::get_linear_velocity() const -> point_f
{
    return to_point_f(b2Body_GetLinearVelocity(ID));
}

void b2d_body::set_linear_velocity(point_f value) const
{
    b2Body_SetLinearVelocity(ID, to_b2Vec2(value));
}

auto b2d_body::get_angular_velocity() const -> radian_f
{
    return radian_f {b2Body_GetAngularVelocity(ID)};
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

auto b2d_body::get_sleep_threshold() const -> f32
{
    return b2Body_GetSleepThreshold(ID);
}

void b2d_body::set_sleep_threshold(f32 value) const
{
    b2Body_SetSleepThreshold(ID, value);
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
    return {{val.p.x, val.p.y}, radian_f {b2Rot_GetAngle(val.q)}};
}

void b2d_body::set_transform(body_transform value) const
{
    b2Body_SetTransform(ID, to_b2Vec2(value.Center), b2MakeRot(value.Angle.Value));
}

auto b2d_body::get_center() const -> point_f
{
    return to_point_f(b2Body_GetWorldCenterOfMass(ID));
}

auto b2d_body::get_local_center() const -> point_f
{
    return to_point_f(b2Body_GetLocalCenterOfMass(ID));
}

auto b2d_body::get_name() const -> string
{
    return helper::to_string(b2Body_GetName(ID));
}

void b2d_body::set_name(string const& value) const
{
    b2Body_SetName(ID, value.c_str());
}

auto b2d_body::get_mass() const -> f32
{
    return b2Body_GetMass(ID);
}

auto b2d_body::get_mass_data() const -> mass_data
{
    auto const md {b2Body_GetMassData(ID)};
    return {.Mass = md.mass, .Center = {md.center.x, md.center.y}, .RotationalInertia = md.rotationalInertia};
}

void b2d_body::set_mass_data(mass_data const& value) const
{
    b2Body_SetMassData(ID, {.mass = value.Mass, .center = to_b2Vec2(value.Center), .rotationalInertia = value.RotationalInertia});
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

void b2d_body::set_user_data(void* ptr) const
{
    b2Body_SetUserData(ID, ptr);
}

void b2d_body::enable_contact_events(bool value) const
{
    b2Body_EnableContactEvents(ID, value);
}

void b2d_body::enable_hit_events(bool value) const
{
    b2Body_EnableHitEvents(ID, value);
}

auto b2d_body::compute_aabb() const -> rect_f
{
    auto const val {b2Body_ComputeAABB(ID)};
    return {{val.lowerBound.x, val.lowerBound.y}, {val.upperBound.x, val.upperBound.y}};
}

auto b2d_body::get_position() const -> point_f
{
    return to_point_f(b2Body_GetPosition(ID));
}

auto b2d_body::get_rotation() const -> radian_f
{
    return radian_f {b2Rot_GetAngle(b2Body_GetRotation(ID))};
}

auto b2d_body::get_local_point(point_f pos) const -> point_f
{
    return to_point_f(b2Body_GetLocalPoint(ID, to_b2Vec2(pos)));
}

auto b2d_body::get_world_point(point_f pos) const -> point_f
{
    return to_point_f(b2Body_GetWorldPoint(ID, to_b2Vec2(pos)));
}

auto b2d_body::get_local_vector(point_f pos) const -> point_f
{
    return to_point_f(b2Body_GetLocalVector(ID, to_b2Vec2(pos)));
}

auto b2d_body::get_world_vector(point_f pos) const -> point_f
{
    return to_point_f(b2Body_GetWorldVector(ID, to_b2Vec2(pos)));
}

auto b2d_body::get_local_point_velocity(point_f pos) const -> point_f
{
    return to_point_f(b2Body_GetLocalPointVelocity(ID, to_b2Vec2(pos)));
}

auto b2d_body::get_world_point_velocity(point_f pos) const -> point_f
{
    return to_point_f(b2Body_GetWorldPointVelocity(ID, to_b2Vec2(pos)));
}

void b2d_body::set_target_transform(body_transform xform, f32 timeStep) const
{
    b2Transform target;
    target.p = to_b2Vec2(xform.Center);
    target.q = b2MakeRot(xform.Angle.Value);

    b2Body_SetTargetTransform(ID, target, timeStep);
}

auto b2d_body::get_rotational_inertia() const -> f32
{
    return b2Body_GetRotationalInertia(ID);
}

void b2d_body::apply_mass_from_shapes() const
{
    b2Body_ApplyMassFromShapes(ID);
}

////////////////////////////////////////////////////////////

b2d_joint::b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, distance_joint::settings const& jointSettings)
{
    auto def {b2DefaultDistanceJointDef()};
    def.bodyIdA          = bodyA->ID;
    def.bodyIdB          = bodyB->ID;
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

b2d_joint::b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, motor_joint::settings const& jointSettings)
{
    auto def {b2DefaultMotorJointDef()};
    def.bodyIdA          = bodyA->ID;
    def.bodyIdB          = bodyB->ID;
    def.collideConnected = jointSettings.IsCollideConnected;
    def.linearOffset     = to_b2Vec2(jointSettings.LinearOffset);
    def.angularOffset    = jointSettings.AngularOffset.Value;
    def.maxForce         = jointSettings.MaxForce;
    def.maxTorque        = jointSettings.MaxTorque;
    def.correctionFactor = jointSettings.CorrectionFactor;

    ID = b2CreateMotorJoint(world->ID, &def);
}

b2d_joint::b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, mouse_joint::settings const& jointSettings)
{
    auto def {b2DefaultMouseJointDef()};
    def.bodyIdA          = bodyA->ID;
    def.bodyIdB          = bodyB->ID;
    def.collideConnected = jointSettings.IsCollideConnected;
    def.hertz            = jointSettings.Hertz;
    def.dampingRatio     = jointSettings.DampingRatio;
    def.maxForce         = jointSettings.MaxForce;

    ID = b2CreateMouseJoint(world->ID, &def);
}

b2d_joint::b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, filter_joint::settings const& /* jointSettings */)
{
    auto def {b2DefaultFilterJointDef()};
    def.bodyIdA = bodyA->ID;
    def.bodyIdB = bodyB->ID;

    ID = b2CreateFilterJoint(world->ID, &def);
}

b2d_joint::b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, prismatic_joint::settings const& jointSettings)
{
    auto def {b2DefaultPrismaticJointDef()};
    def.bodyIdA          = bodyA->ID;
    def.bodyIdB          = bodyB->ID;
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

b2d_joint::b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, revolute_joint::settings const& jointSettings)
{
    auto def {b2DefaultRevoluteJointDef()};
    def.bodyIdA          = bodyA->ID;
    def.bodyIdB          = bodyB->ID;
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

b2d_joint::b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, weld_joint::settings const& jointSettings)
{
    auto def {b2DefaultWeldJointDef()};
    def.bodyIdA             = bodyA->ID;
    def.bodyIdB             = bodyB->ID;
    def.collideConnected    = jointSettings.IsCollideConnected;
    def.referenceAngle      = jointSettings.ReferenceAngle.Value;
    def.linearHertz         = jointSettings.LinearHertz;
    def.angularHertz        = jointSettings.AngularHertz;
    def.linearDampingRatio  = jointSettings.LinearDampingRatio;
    def.angularDampingRatio = jointSettings.AngularDampingRatio;

    ID = b2CreateWeldJoint(world->ID, &def);
}

b2d_joint::b2d_joint(b2d_world* world, b2d_body const* bodyA, b2d_body const* bodyB, wheel_joint::settings const& jointSettings)
{
    auto def {b2DefaultWheelJointDef()};
    def.bodyIdA          = bodyA->ID;
    def.bodyIdB          = bodyB->ID;
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

void b2d_joint::set_user_data(void* ptr) const
{
    b2Joint_SetUserData(ID, ptr);
}

auto b2d_joint::get_body_a() const -> body*
{
    return get_body(b2Joint_GetBodyA(ID));
}

auto b2d_joint::get_body_b() const -> body*
{
    return get_body(b2Joint_GetBodyB(ID));
}

void b2d_joint::wake_bodies() const
{
    b2Joint_WakeBodies(ID);
}

auto b2d_joint::get_local_anchor_a() const -> point_f
{
    return to_point_f(b2Joint_GetLocalAnchorA(ID));
}

auto b2d_joint::get_local_anchor_b() const -> point_f
{
    return to_point_f(b2Joint_GetLocalAnchorB(ID));
}

auto b2d_joint::get_constraint_force() const -> point_f
{
    return to_point_f(b2Joint_GetConstraintForce(ID));
}

auto b2d_joint::get_constraint_torque() const -> f32
{
    return b2Joint_GetConstraintTorque(ID);
}

auto b2d_joint::get_collide_connected() const -> bool
{
    return b2Joint_GetCollideConnected(ID);
}

void b2d_joint::set_collide_connected(bool value) const
{
    b2Joint_SetCollideConnected(ID, value);
}

void b2d_joint::distance_joint_set_length(f32 length) const
{
    b2DistanceJoint_SetLength(ID, length);
}

auto b2d_joint::distance_joint_get_length() const -> f32
{
    return b2DistanceJoint_GetLength(ID);
}

void b2d_joint::distance_joint_enable_spring(bool enableSpring) const
{
    b2DistanceJoint_EnableSpring(ID, enableSpring);
}

auto b2d_joint::distance_joint_is_spring_enabled() const -> bool
{
    return b2DistanceJoint_IsSpringEnabled(ID);
}

void b2d_joint::distance_joint_set_spring_hertz(f32 hertz) const
{
    b2DistanceJoint_SetSpringHertz(ID, hertz);
}

auto b2d_joint::distance_joint_get_spring_hertz() const -> f32
{
    return b2DistanceJoint_GetSpringHertz(ID);
}

void b2d_joint::distance_joint_set_spring_damping_ratio(f32 dampingRatio) const
{
    b2DistanceJoint_SetSpringDampingRatio(ID, dampingRatio);
}

auto b2d_joint::distance_joint_get_spring_damping_ratio() const -> f32
{
    return b2DistanceJoint_GetSpringDampingRatio(ID);
}

void b2d_joint::distance_joint_enable_limit(bool enableLimit) const
{
    b2DistanceJoint_EnableLimit(ID, enableLimit);
}

auto b2d_joint::distance_joint_is_limit_enabled() const -> bool
{
    return b2DistanceJoint_IsLimitEnabled(ID);
}

void b2d_joint::distance_joint_set_length_range(f32 minLength, f32 maxLength) const
{
    b2DistanceJoint_SetLengthRange(ID, minLength, maxLength);
}

auto b2d_joint::distance_joint_get_min_length() const -> f32
{
    return b2DistanceJoint_GetMinLength(ID);
}

auto b2d_joint::distance_joint_get_max_length() const -> f32
{
    return b2DistanceJoint_GetMaxLength(ID);
}

auto b2d_joint::distance_joint_get_current_length() const -> f32
{
    return b2DistanceJoint_GetCurrentLength(ID);
}

void b2d_joint::distance_joint_enable_motor(bool enableMotor) const
{
    b2DistanceJoint_EnableMotor(ID, enableMotor);
}

auto b2d_joint::distance_joint_is_motor_enabled() const -> bool
{
    return b2DistanceJoint_IsMotorEnabled(ID);
}

void b2d_joint::distance_joint_set_motor_speed(f32 motorSpeed) const
{
    b2DistanceJoint_SetMotorSpeed(ID, motorSpeed);
}

auto b2d_joint::distance_joint_get_motor_speed() const -> f32
{
    return b2DistanceJoint_GetMotorSpeed(ID);
}

void b2d_joint::distance_joint_set_max_motor_force(f32 force) const
{
    b2DistanceJoint_SetMaxMotorForce(ID, force);
}

auto b2d_joint::distance_joint_get_max_motor_force() const -> f32
{
    return b2DistanceJoint_GetMaxMotorForce(ID);
}

auto b2d_joint::distance_joint_get_motor_force() const -> f32
{
    return b2DistanceJoint_GetMotorForce(ID);
}

void b2d_joint::motor_joint_set_linear_offset(point_f linearOffset) const
{
    b2MotorJoint_SetLinearOffset(ID, to_b2Vec2(linearOffset));
}

auto b2d_joint::motor_joint_get_linear_offset() const -> point_f
{
    return to_point_f(b2MotorJoint_GetLinearOffset(ID));
}

void b2d_joint::motor_joint_set_angular_offset(f32 angularOffset) const
{
    b2MotorJoint_SetAngularOffset(ID, angularOffset);
}

auto b2d_joint::motor_joint_get_angular_offset() const -> f32
{
    return b2MotorJoint_GetAngularOffset(ID);
}

void b2d_joint::motor_joint_set_max_force(f32 maxForce) const
{
    b2MotorJoint_SetMaxForce(ID, maxForce);
}

auto b2d_joint::motor_joint_get_max_force() const -> f32
{
    return b2MotorJoint_GetMaxForce(ID);
}

void b2d_joint::motor_joint_set_max_torque(f32 maxTorque) const
{
    b2MotorJoint_SetMaxTorque(ID, maxTorque);
}

auto b2d_joint::motor_joint_get_max_torque() const -> f32
{
    return b2MotorJoint_GetMaxTorque(ID);
}

void b2d_joint::motor_joint_set_correction_factor(f32 correctionFactor) const
{
    b2MotorJoint_SetCorrectionFactor(ID, correctionFactor);
}

auto b2d_joint::motor_joint_get_correction_factor() const -> f32
{
    return b2MotorJoint_GetCorrectionFactor(ID);
}

void b2d_joint::mouse_joint_set_target(point_f target) const
{
    b2MouseJoint_SetTarget(ID, to_b2Vec2(target));
}

auto b2d_joint::mouse_joint_get_target() const -> point_f
{
    return to_point_f(b2MouseJoint_GetTarget(ID));
}

void b2d_joint::mouse_joint_set_spring_hertz(f32 hertz) const
{
    b2MouseJoint_SetSpringHertz(ID, hertz);
}

auto b2d_joint::mouse_joint_get_spring_hertz() const -> f32
{
    return b2MouseJoint_GetSpringHertz(ID);
}

void b2d_joint::mouse_joint_set_spring_damping_ratio(f32 dampingRatio) const
{
    b2MouseJoint_SetSpringDampingRatio(ID, dampingRatio);
}

auto b2d_joint::mouse_joint_get_spring_damping_ratio() const -> f32
{
    return b2MouseJoint_GetSpringDampingRatio(ID);
}

void b2d_joint::mouse_joint_set_max_force(f32 maxForce) const
{
    b2MouseJoint_SetMaxForce(ID, maxForce);
}

auto b2d_joint::mouse_joint_get_max_force() const -> f32
{
    return b2MouseJoint_GetMaxForce(ID);
}

void b2d_joint::prismatic_joint_enable_spring(bool enableSpring) const
{
    b2PrismaticJoint_EnableSpring(ID, enableSpring);
}

auto b2d_joint::prismatic_joint_is_spring_enabled() const -> bool
{
    return b2PrismaticJoint_IsSpringEnabled(ID);
}

auto b2d_joint::prismatic_joint_get_spring_hertz() const -> f32
{
    return b2PrismaticJoint_GetSpringHertz(ID);
}

void b2d_joint::prismatic_joint_set_spring_hertz(f32 hertz) const
{
    b2PrismaticJoint_SetSpringHertz(ID, hertz);
}

auto b2d_joint::prismatic_joint_get_spring_damping_ratio() const -> f32
{
    return b2PrismaticJoint_GetSpringDampingRatio(ID);
}

void b2d_joint::prismatic_joint_set_spring_damping_ratio(f32 dampingRatio) const
{
    b2PrismaticJoint_SetSpringDampingRatio(ID, dampingRatio);
}

void b2d_joint::prismatic_joint_enable_limit(bool enableLimit) const
{
    b2PrismaticJoint_EnableLimit(ID, enableLimit);
}

auto b2d_joint::prismatic_joint_is_limit_enabled() const -> bool
{
    return b2PrismaticJoint_IsLimitEnabled(ID);
}

auto b2d_joint::prismatic_joint_get_lower_limit() const -> f32
{
    return b2PrismaticJoint_GetLowerLimit(ID);
}

auto b2d_joint::prismatic_joint_get_upper_limit() const -> f32
{
    return b2PrismaticJoint_GetUpperLimit(ID);
}

void b2d_joint::prismatic_joint_set_limits(f32 lower, f32 upper) const
{
    b2PrismaticJoint_SetLimits(ID, lower, upper);
}

void b2d_joint::prismatic_joint_enable_motor(bool enableMotor) const
{
    b2PrismaticJoint_EnableMotor(ID, enableMotor);
}

auto b2d_joint::prismatic_joint_is_motor_enabled() const -> bool
{
    return b2PrismaticJoint_IsMotorEnabled(ID);
}

void b2d_joint::prismatic_joint_set_motor_speed(f32 motorSpeed) const
{
    b2PrismaticJoint_SetMotorSpeed(ID, motorSpeed);
}

auto b2d_joint::prismatic_joint_get_motor_speed() const -> f32
{
    return b2PrismaticJoint_GetMotorSpeed(ID);
}

void b2d_joint::prismatic_joint_set_max_motor_force(f32 force) const
{
    b2PrismaticJoint_SetMaxMotorForce(ID, force);
}

auto b2d_joint::prismatic_joint_get_max_motor_force() const -> f32
{
    return b2PrismaticJoint_GetMaxMotorForce(ID);
}

auto b2d_joint::prismatic_joint_get_motor_force() const -> f32
{
    return b2PrismaticJoint_GetMotorForce(ID);
}

auto b2d_joint::prismatic_joint_get_translation() const -> f32
{
    return b2PrismaticJoint_GetTranslation(ID);
}

auto b2d_joint::prismatic_joint_get_speed() const -> f32
{
    return b2PrismaticJoint_GetSpeed(ID);
}

void b2d_joint::revolute_joint_enable_spring(bool enableSpring) const
{
    b2RevoluteJoint_EnableSpring(ID, enableSpring);
}

auto b2d_joint::revolute_joint_is_spring_enabled() const -> bool
{
    return b2RevoluteJoint_IsSpringEnabled(ID);
}

void b2d_joint::revolute_joint_set_spring_hertz(f32 hertz) const
{
    b2RevoluteJoint_SetSpringHertz(ID, hertz);
}

auto b2d_joint::revolute_joint_get_spring_hertz() const -> f32
{
    return b2RevoluteJoint_GetSpringHertz(ID);
}

void b2d_joint::revolute_joint_set_spring_damping_ratio(f32 dampingRatio) const
{
    b2RevoluteJoint_SetSpringDampingRatio(ID, dampingRatio);
}

auto b2d_joint::revolute_joint_get_spring_damping_ratio() const -> f32
{
    return b2RevoluteJoint_GetSpringDampingRatio(ID);
}

auto b2d_joint::revolute_joint_get_angle() const -> radian_f
{
    return radian_f {b2RevoluteJoint_GetAngle(ID)};
}

void b2d_joint::revolute_joint_enable_limit(bool enableLimit) const
{
    b2RevoluteJoint_EnableLimit(ID, enableLimit);
}

auto b2d_joint::revolute_joint_is_limit_enabled() const -> bool
{
    return b2RevoluteJoint_IsLimitEnabled(ID);
}

auto b2d_joint::revolute_joint_get_lower_limit() const -> f32
{
    return b2RevoluteJoint_GetLowerLimit(ID);
}

auto b2d_joint::revolute_joint_get_upper_limit() const -> f32
{
    return b2RevoluteJoint_GetUpperLimit(ID);
}

void b2d_joint::revolute_joint_set_limits(f32 lower, f32 upper) const
{
    b2RevoluteJoint_SetLimits(ID, lower, upper);
}

void b2d_joint::revolute_joint_enable_motor(bool enableMotor) const
{
    b2RevoluteJoint_EnableMotor(ID, enableMotor);
}

auto b2d_joint::revolute_joint_is_motor_enabled() const -> bool
{
    return b2RevoluteJoint_IsMotorEnabled(ID);
}

void b2d_joint::revolute_joint_set_motor_speed(f32 motorSpeed) const
{
    b2RevoluteJoint_SetMotorSpeed(ID, motorSpeed);
}

auto b2d_joint::revolute_joint_get_motor_speed() const -> f32
{
    return b2RevoluteJoint_GetMotorSpeed(ID);
}

auto b2d_joint::revolute_joint_get_motor_torque() const -> f32
{
    return b2RevoluteJoint_GetMotorTorque(ID);
}

void b2d_joint::revolute_joint_set_max_motor_torque(f32 torque) const
{
    b2RevoluteJoint_SetMaxMotorTorque(ID, torque);
}

auto b2d_joint::revolute_joint_get_max_motor_torque() const -> f32
{
    return b2RevoluteJoint_GetMaxMotorTorque(ID);
}

void b2d_joint::weld_joint_set_linear_hertz(f32 hertz) const
{
    b2WeldJoint_SetLinearHertz(ID, hertz);
}

auto b2d_joint::weld_joint_get_linear_hertz() const -> f32
{
    return b2WeldJoint_GetLinearHertz(ID);
}

void b2d_joint::weld_joint_set_linear_damping_ratio(f32 dampingRatio) const
{
    b2WeldJoint_SetLinearDampingRatio(ID, dampingRatio);
}

auto b2d_joint::weld_joint_get_linear_damping_ratio() const -> f32
{
    return b2WeldJoint_GetLinearDampingRatio(ID);
}

void b2d_joint::weld_joint_set_angular_hertz(f32 hertz) const
{
    b2WeldJoint_SetAngularHertz(ID, hertz);
}

auto b2d_joint::weld_joint_get_angular_hertz() const -> f32
{
    return b2WeldJoint_GetAngularHertz(ID);
}

void b2d_joint::weld_joint_set_angular_damping_ratio(f32 dampingRatio) const
{
    b2WeldJoint_SetAngularDampingRatio(ID, dampingRatio);
}

auto b2d_joint::weld_joint_get_angular_damping_ratio() const -> f32
{
    return b2WeldJoint_GetAngularDampingRatio(ID);
}

void b2d_joint::wheel_joint_enable_spring(bool enableSpring) const
{
    b2WheelJoint_EnableSpring(ID, enableSpring);
}

auto b2d_joint::wheel_joint_is_spring_enabled() const -> bool
{
    return b2WheelJoint_IsSpringEnabled(ID);
}

void b2d_joint::wheel_joint_set_spring_hertz(f32 hertz) const
{
    b2WheelJoint_SetSpringHertz(ID, hertz);
}

auto b2d_joint::wheel_joint_get_spring_hertz() const -> f32
{
    return b2WheelJoint_GetSpringHertz(ID);
}

void b2d_joint::wheel_joint_set_spring_damping_ratio(f32 dampingRatio) const
{
    b2WheelJoint_SetSpringDampingRatio(ID, dampingRatio);
}

auto b2d_joint::wheel_joint_get_spring_damping_ratio() const -> f32
{
    return b2WheelJoint_GetSpringDampingRatio(ID);
}

void b2d_joint::wheel_joint_enable_limit(bool enableLimit) const
{
    b2WheelJoint_EnableLimit(ID, enableLimit);
}

auto b2d_joint::wheel_joint_is_limit_enabled() const -> bool
{
    return b2WheelJoint_IsLimitEnabled(ID);
}

auto b2d_joint::wheel_joint_get_lower_limit() const -> f32
{
    return b2WheelJoint_GetLowerLimit(ID);
}

auto b2d_joint::wheel_joint_get_upper_limit() const -> f32
{
    return b2WheelJoint_GetUpperLimit(ID);
}

void b2d_joint::wheel_joint_set_limits(f32 lower, f32 upper) const
{
    b2WheelJoint_SetLimits(ID, lower, upper);
}

void b2d_joint::wheel_joint_enable_motor(bool enableMotor) const
{
    b2WheelJoint_EnableMotor(ID, enableMotor);
}

auto b2d_joint::wheel_joint_is_motor_enabled() const -> bool
{
    return b2WheelJoint_IsMotorEnabled(ID);
}

void b2d_joint::wheel_joint_set_motor_speed(f32 motorSpeed) const
{
    b2WheelJoint_SetMotorSpeed(ID, motorSpeed);
}

auto b2d_joint::wheel_joint_get_motor_speed() const -> f32
{
    return b2WheelJoint_GetMotorSpeed(ID);
}

void b2d_joint::wheel_joint_set_max_motor_torque(f32 torque) const
{
    b2WheelJoint_SetMaxMotorTorque(ID, torque);
}

auto b2d_joint::wheel_joint_get_max_motor_torque() const -> f32
{
    return b2WheelJoint_GetMaxMotorTorque(ID);
}

auto b2d_joint::wheel_joint_get_motor_torque() const -> f32
{
    return b2WheelJoint_GetMotorTorque(ID);
}

////////////////////////////////////////////////////////////

auto static GetShapeDef(shape::settings const& shapeSettings)
{
    b2ShapeDef shapeDef {b2DefaultShapeDef()};
    shapeDef.material.friction          = shapeSettings.Material.Friction;
    shapeDef.material.restitution       = shapeSettings.Material.Restitution;
    shapeDef.material.rollingResistance = shapeSettings.Material.RollingResistance;
    shapeDef.material.tangentSpeed      = shapeSettings.Material.TangentSpeed;
    shapeDef.material.customColor       = static_cast<u32>(shapeSettings.Material.CustomColor.R << 16 | shapeSettings.Material.CustomColor.G << 8 | shapeSettings.Material.CustomColor.B);
    shapeDef.density                    = shapeSettings.Density;
    shapeDef.filter.categoryBits        = shapeSettings.Filter.CategoryBits;
    shapeDef.filter.maskBits            = shapeSettings.Filter.MaskBits;
    shapeDef.filter.groupIndex          = shapeSettings.Filter.GroupIndex;
    shapeDef.isSensor                   = shapeSettings.IsSensor;
    shapeDef.enableSensorEvents         = shapeSettings.EnableSensorEvents;
    shapeDef.enableContactEvents        = shapeSettings.EnableContactEvents;
    shapeDef.enableHitEvents            = shapeSettings.EnableHitEvents;
    shapeDef.enablePreSolveEvents       = shapeSettings.EnablePreSolveEvents;
    shapeDef.invokeContactCreation      = shapeSettings.InvokeContactCreation;
    shapeDef.updateBodyMass             = shapeSettings.UpdateBodyMass;
    return shapeDef;
}

b2d_shape::b2d_shape(b2d_body* body, polygon_shape::settings const& settings, shape::settings const& shapeSettings)
{
    std::vector<b2Vec2> v;
    v.reserve(settings.Verts.size());
    for (auto const& vert : settings.Verts) {
        v.emplace_back(vert.X, vert.Y);
    }

    b2Hull h {b2ComputeHull(v.data(), static_cast<i32>(v.size()))};
    auto   poly {b2MakePolygon(&h, settings.Radius)};

    b2ShapeDef shapeDef {GetShapeDef(shapeSettings)};
    ID = b2CreatePolygonShape(body->ID, &shapeDef, &poly);
}

b2d_shape::~b2d_shape()
{
    b2DestroyShape(ID, true);
}

b2d_shape::b2d_shape(b2d_body* body, rect_shape::settings const& settings, shape::settings const& shapeSettings)
{
    auto const& rect {settings.Extents};
    auto        poly {b2MakeOffsetBox(rect.width() / 2, rect.height() / 2, to_b2Vec2(rect.top_left()), b2MakeRot(settings.Angle.Value))};

    b2ShapeDef shapeDef {GetShapeDef(shapeSettings)};
    ID = b2CreatePolygonShape(body->ID, &shapeDef, &poly);
}

b2d_shape::b2d_shape(b2d_body* body, circle_shape::settings const& settings, shape::settings const& shapeSettings)
{
    b2Circle poly {to_b2Vec2(settings.Center), settings.Radius};

    b2ShapeDef shapeDef {GetShapeDef(shapeSettings)};
    ID = b2CreateCircleShape(body->ID, &shapeDef, &poly);
}

b2d_shape::b2d_shape(b2d_body* body, segment_shape::settings const& settings, shape::settings const& shapeSettings)
{
    b2Segment poly {to_b2Vec2(settings.Point1), to_b2Vec2(settings.Point2)};

    b2ShapeDef shapeDef {GetShapeDef(shapeSettings)};
    ID = b2CreateSegmentShape(body->ID, &shapeDef, &poly);
}

b2d_shape::b2d_shape(b2d_body* body, capsule_shape::settings const& settings, shape::settings const& shapeSettings)
{
    b2Capsule poly {to_b2Vec2(settings.Center0), to_b2Vec2(settings.Center1), settings.Radius};

    b2ShapeDef shapeDef {GetShapeDef(shapeSettings)};
    ID = b2CreateCapsuleShape(body->ID, &shapeDef, &poly);
}

auto b2d_shape::is_sensor() const -> bool
{
    return b2Shape_IsSensor(ID);
}

auto b2d_shape::get_density() const -> f32
{
    return b2Shape_GetDensity(ID);
}

void b2d_shape::set_density(f32 density) const
{
    b2Shape_SetDensity(ID, density, true);
}

auto b2d_shape::get_friction() const -> f32
{
    return b2Shape_GetFriction(ID);
}

void b2d_shape::set_friction(f32 friction) const
{
    b2Shape_SetFriction(ID, friction);
}

auto b2d_shape::get_restitution() const -> f32
{
    return b2Shape_GetRestitution(ID);
}

void b2d_shape::set_restitution(f32 restitution) const
{
    b2Shape_SetRestitution(ID, restitution);
}

void b2d_shape::enable_sensor_events(bool flag) const
{
    b2Shape_EnableSensorEvents(ID, flag);
}

auto b2d_shape::are_sensor_events_enabled() const -> bool
{
    return b2Shape_AreSensorEventsEnabled(ID);
}

void b2d_shape::enable_contact_events(bool flag) const
{
    b2Shape_EnableContactEvents(ID, flag);
}

auto b2d_shape::are_contact_events_enabled() const -> bool
{
    return b2Shape_AreContactEventsEnabled(ID);
}

void b2d_shape::enable_pre_solve_events(bool flag) const
{
    b2Shape_EnablePreSolveEvents(ID, flag);
}

auto b2d_shape::are_pre_solve_events_enabled() const -> bool
{
    return b2Shape_ArePreSolveEventsEnabled(ID);
}

void b2d_shape::enable_hit_events(bool flag) const
{
    b2Shape_EnableHitEvents(ID, flag);
}

auto b2d_shape::are_hit_events_enabled() const -> bool
{
    return b2Shape_AreHitEventsEnabled(ID);
}

auto b2d_shape::test_point(point_f point) const -> bool
{
    return b2Shape_TestPoint(ID, to_b2Vec2(point));
}

auto b2d_shape::get_aabb() const -> AABB
{
    auto const val {b2Shape_GetAABB(ID)};
    return {{val.lowerBound.x, val.lowerBound.y}, {val.upperBound.x, val.upperBound.y}};
}

auto b2d_shape::get_closest_point(point_f target) const -> point_f
{
    return to_point_f(b2Shape_GetClosestPoint(ID, to_b2Vec2(target)));
}

void b2d_shape::set_user_data(void* ptr) const
{
    b2Shape_SetUserData(ID, ptr);
}

auto b2d_shape::equal(b2d_shape const* other) const -> bool
{
    return B2_ID_EQUALS(ID, other->ID);
}

auto b2d_shape::get_mass_data() const -> mass_data
{
    auto const md {b2Shape_GetMassData(ID)};
    return {.Mass = md.mass, .Center = {md.center.x, md.center.y}, .RotationalInertia = md.rotationalInertia};
}

auto b2d_shape::get_filter() const -> filter
{
    auto const f {b2Shape_GetFilter(ID)};
    return {.CategoryBits = f.categoryBits, .MaskBits = f.maskBits, .GroupIndex = f.groupIndex};
}

void b2d_shape::set_filter(filter const& value) const
{
    b2Shape_SetFilter(ID, {.categoryBits = value.CategoryBits, .maskBits = value.MaskBits, .groupIndex = value.GroupIndex});
}

auto b2d_shape::get_sensor_overlaps() const -> std::vector<shape*>
{
    i32 const cap {b2Shape_GetSensorCapacity(ID)};
    if (cap == 0) { return {}; }

    std::vector<b2ShapeId> shapes(cap);
    i32 const              ret {b2Shape_GetSensorOverlaps(ID, shapes.data(), static_cast<i32>(shapes.size()))};
    if (ret == 0) { return {}; }

    std::vector<shape*> retValue;
    retValue.reserve(ret);
    for (i32 i {0}; i < ret; ++i) {
        auto const& id {shapes[i]};
        if (b2Shape_IsValid(id)) {
            retValue.push_back(get_shape(id));
        }
    }
    return retValue;
}

////////////////////////////////////////////////////////////

b2d_chain::b2d_chain(b2d_body* body, chain::settings const& shapeSettings)
{
    b2ChainDef shapeDef {b2DefaultChainDef()};

    std::vector<b2Vec2> points;
    points.reserve(shapeSettings.Points.size());
    for (auto const p : shapeSettings.Points) {
        points.push_back(to_b2Vec2(p));
    }
    shapeDef.points = points.data();
    shapeDef.count  = static_cast<i32>(shapeSettings.Points.size());

    std::vector<b2SurfaceMaterial> materials;
    materials.reserve(shapeSettings.Materials.size());
    for (auto const mat : shapeSettings.Materials) {
        auto& v {materials.emplace_back()};
        v.friction          = mat.Friction;
        v.restitution       = mat.Restitution;
        v.rollingResistance = mat.RollingResistance;
        v.tangentSpeed      = mat.TangentSpeed;
        v.customColor       = static_cast<u32>(mat.CustomColor.R << 16 | mat.CustomColor.G << 8 | mat.CustomColor.B);
    }
    shapeDef.materials     = materials.data();
    shapeDef.materialCount = static_cast<i32>(shapeSettings.Materials.size());

    shapeDef.filter.categoryBits = shapeSettings.Filter.CategoryBits;
    shapeDef.filter.maskBits     = shapeSettings.Filter.MaskBits;
    shapeDef.filter.groupIndex   = shapeSettings.Filter.GroupIndex;

    shapeDef.isLoop             = shapeSettings.IsLoop;
    shapeDef.enableSensorEvents = shapeSettings.EnableSensorEvents;
    ID                          = b2CreateChain(body->ID, &shapeDef);
}

b2d_chain::~b2d_chain()
{
    b2DestroyChain(ID);
}

auto b2d_chain::get_friction() const -> f32
{
    return b2Chain_GetFriction(ID);
}

void b2d_chain::set_friction(f32 value) const
{
    b2Chain_SetFriction(ID, value);
}

auto b2d_chain::get_restitution() const -> f32
{
    return b2Chain_GetRestitution(ID);
}

void b2d_chain::set_restitution(f32 value) const
{
    b2Chain_SetRestitution(ID, value);
}

auto b2d_chain::get_segments() const -> std::vector<chain_segment>
{
    auto const             count {b2Chain_GetSegmentCount(ID)};
    std::vector<b2ShapeId> shapes(count);
    auto const             ret {b2Chain_GetSegments(ID, shapes.data(), count)};
    if (ret == 0) { return {}; }

    std::vector<chain_segment> retValue;
    retValue.reserve(ret);
    for (i32 i {0}; i < ret; ++i) {
        auto& id {shapes[i]};
        if (b2Shape_IsValid(id)) {
            auto const shape {b2Shape_GetChainSegment(id)};
            auto&      cs {retValue.emplace_back()};
            cs.Segment.Point1 = to_point_f(shape.segment.point1);
            cs.Segment.Point2 = to_point_f(shape.segment.point2);
            cs.Ghost1         = to_point_f(shape.ghost1);
            cs.Ghost2         = to_point_f(shape.ghost2);
        }
    }
    return retValue;
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

    ddraw->Impl->draw_polygon(verts, color::FromRGB(color));
}

void static DrawSolidPolygon(b2Transform transform, b2Vec2 const* vertices, int vertexCount, float radius, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};

    std::span<b2Vec2 const> span {vertices, static_cast<usize>(vertexCount)};
    std::vector<point_f>    verts;
    verts.reserve(vertexCount);
    for (auto const& v : span) { verts.emplace_back(v.x, v.y); }

    ddraw->Impl->draw_solid_polygon({.Center = {transform.p.x, transform.p.y}, .Angle = radian_f {b2Rot_GetAngle(transform.q)}}, verts, radius, color::FromRGB(color));
}

void static DrawCircle(b2Vec2 center, float radius, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->Impl->draw_circle({center.x, center.y}, radius, color::FromRGB(color));
}

void static DrawSolidCircle(b2Transform transform, float radius, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->Impl->draw_solid_circle({.Center = {transform.p.x, transform.p.y}, .Angle = radian_f {b2Rot_GetAngle(transform.q)}}, radius, color::FromRGB(color));
}

void static DrawSolidCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->Impl->draw_solid_capsule({p1.x, p1.y}, {p2.x, p2.y}, radius, color::FromRGB(color));
}

void static DrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->Impl->draw_segment({p1.x, p1.y}, {p2.x, p2.y}, color::FromRGB(color));
}

void static DrawTransform(b2Transform transform, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->Impl->draw_transform({.Center = {transform.p.x, transform.p.y}, .Angle = radian_f {b2Rot_GetAngle(transform.q)}});
}

void static DrawPoint(b2Vec2 p, float size, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->Impl->draw_point({p.x, p.y}, size, color::FromRGB(color));
}

void static DrawString(b2Vec2 p, char const* s, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->Impl->draw_string({p.x, p.y}, s, color::FromRGB(color));
}
}

b2d_debug_draw::b2d_debug_draw(debug_draw* impl)
    : Impl {impl}
{
    ID.context             = this;
    ID.DrawPolygonFcn      = &DrawPolygon;
    ID.DrawSolidPolygonFcn = &DrawSolidPolygon;
    ID.DrawCircleFcn       = &DrawCircle;
    ID.DrawSolidCircleFcn  = &DrawSolidCircle;
    ID.DrawSolidCapsuleFcn = &DrawSolidCapsule;
    ID.DrawSegmentFcn      = &DrawSegment;
    ID.DrawTransformFcn    = &DrawTransform;
    ID.DrawPointFcn        = &DrawPoint;
    ID.DrawStringFcn       = &DrawString;
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
    ID.drawBounds           = settings.DrawBounds;
    ID.drawMass             = settings.DrawMass;
    ID.drawBodyNames        = settings.DrawBodyNames;
    ID.drawContacts         = settings.DrawContacts;
    ID.drawGraphColors      = settings.DrawGraphColors;
    ID.drawContactNormals   = settings.DrawContactNormals;
    ID.drawContactImpulses  = settings.DrawContactImpulses;
    ID.drawContactFeatures  = settings.DrawContactFeatures;
    ID.drawFrictionImpulses = settings.DrawFrictionImpulses;
    ID.drawIslands          = settings.DrawIslands;
}

////////////////////////////////////////////////////////////

auto rot_from_angle(radian_f angle) -> rotation
{
    auto const rot {b2MakeRot(angle.Value)};
    return {.Cosine = rot.c, .Sine = rot.s};
}
}

#endif
