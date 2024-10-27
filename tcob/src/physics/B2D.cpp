// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "B2D.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include "tcob/core/ServiceLocator.hpp"
    #include "tcob/core/TaskManager.hpp"

B2_API auto b2RevoluteJoint_IsSpringEnabled(b2JointId jointId) -> bool;

namespace tcob::physics::detail {
auto static to_b2Vec2(point_f val) -> b2Vec2
{
    return {val.X, val.Y};
}

b2d_world::b2d_world(world::settings const& settings)
{
    b2WorldDef worldDef {b2DefaultWorldDef()};
    worldDef.gravity               = to_b2Vec2(settings.Gravity);
    worldDef.restitutionThreshold  = settings.RestitutionThreshold;
    worldDef.contactDampingRatio   = settings.ContactPushoutVelocity;
    worldDef.hitEventThreshold     = settings.HitEventThreshold;
    worldDef.contactHertz          = settings.ContactHertz;
    worldDef.contactDampingRatio   = settings.ContactDampingRatio;
    worldDef.jointHertz            = settings.JointHertz;
    worldDef.jointDampingRatio     = settings.JointDampingRatio;
    worldDef.maximumLinearVelocity = settings.MaximumLinearVelocity;
    worldDef.enableSleep           = settings.EnableSleeping;
    worldDef.enableContinous       = settings.EnableContinuous;

    worldDef.workerCount = static_cast<i32>(locate_service<task_manager>().get_thread_count());
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
    auto const val {b2World_GetGravity(ID)};
    return {val.x, val.y};
}

void b2d_world::set_gravity(point_f value) const
{
    b2World_SetGravity(ID, to_b2Vec2(value));
}

void b2d_world::set_enable_sleeping(bool value) const
{
    b2World_EnableSleeping(ID, value);
}

void b2d_world::set_enable_continuous(bool value) const
{
    b2World_EnableContinuous(ID, value);
}

void b2d_world::set_restitution_threshold(f32 value) const
{
    b2World_SetRestitutionThreshold(ID, value);
}

void b2d_world::set_hit_event_threshold(f32 value) const
{
    b2World_SetHitEventThreshold(ID, value);
}

void b2d_world::explode(point_f pos, f32 radius, f32 impulse) const
{
    b2World_Explode(ID, to_b2Vec2(pos), radius, impulse);
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
        ev.Body       = reinterpret_cast<body*>(b2Body_GetUserData(event.bodyId));
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
        ev.ShapeA = reinterpret_cast<shape*>(b2Shape_GetUserData(event.shapeIdA));
        ev.ShapeB = reinterpret_cast<shape*>(b2Shape_GetUserData(event.shapeIdB));
        retValue.BeginTouch.push_back(ev);
    }
    std::span<b2ContactEndTouchEvent> const end {events.endEvents, static_cast<usize>(events.endCount)};
    for (auto& event : end) {
        contact_end_touch_event ev;
        ev.ShapeA = reinterpret_cast<shape*>(b2Shape_GetUserData(event.shapeIdA));
        ev.ShapeB = reinterpret_cast<shape*>(b2Shape_GetUserData(event.shapeIdB));
        retValue.EndTouch.push_back(ev);
    }

    std::span<b2ContactHitEvent> const hit {events.hitEvents, static_cast<usize>(events.hitCount)};
    for (auto& event : hit) {
        contact_hit_event ev;
        ev.ShapeA        = reinterpret_cast<shape*>(b2Shape_GetUserData(event.shapeIdA));
        ev.ShapeB        = reinterpret_cast<shape*>(b2Shape_GetUserData(event.shapeIdB));
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
        sensor_begin_touch_event ev;
        ev.Sensor  = reinterpret_cast<shape*>(b2Shape_GetUserData(event.sensorShapeId));
        ev.Visitor = reinterpret_cast<shape*>(b2Shape_GetUserData(event.visitorShapeId));
        retValue.BeginTouch.push_back(ev);
    }
    std::span<b2SensorEndTouchEvent> const end {events.endEvents, static_cast<usize>(events.endCount)};
    for (auto& event : end) {
        sensor_end_touch_event ev;
        ev.Sensor  = reinterpret_cast<shape*>(b2Shape_GetUserData(event.sensorShapeId));
        ev.Visitor = reinterpret_cast<shape*>(b2Shape_GetUserData(event.visitorShapeId));
        retValue.EndTouch.push_back(ev);
    }

    return retValue;
}

////////////////////////////////////////////////////////////

b2d_body::b2d_body(b2d_world* world, body_transform const& xform, body::settings const& bodySettings)
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

void b2d_body::set_user_data(void* ptr) const
{
    b2Body_SetUserData(ID, ptr);
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

auto b2d_joint::distance_joint_get_hertz() const -> f32
{
    return b2DistanceJoint_GetHertz(ID);
}

void b2d_joint::distance_joint_set_spring_damping_ratio(f32 dampingRatio) const
{
    b2DistanceJoint_SetSpringDampingRatio(ID, dampingRatio);
}

auto b2d_joint::distance_joint_get_damping_ratio() const -> f32
{
    return b2DistanceJoint_GetDampingRatio(ID);
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
    auto const val {b2MotorJoint_GetLinearOffset(ID)};
    return {val.x, val.y};
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
    auto const val {b2MouseJoint_GetTarget(ID)};
    return {val.x, val.y};
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

void b2d_joint::prismatic_joint_set_spring_hertz(f32 hertz) const
{
    b2PrismaticJoint_SetSpringHertz(ID, hertz);
}

auto b2d_joint::prismatic_joint_get_spring_hertz() const -> f32
{
    return b2PrismaticJoint_GetSpringHertz(ID);
}

void b2d_joint::prismatic_joint_set_spring_damping_ratio(f32 dampingRatio) const
{
    b2PrismaticJoint_SetSpringDampingRatio(ID, dampingRatio);
}

auto b2d_joint::prismatic_joint_get_spring_damping_ratio() const -> f32
{
    return b2PrismaticJoint_GetSpringDampingRatio(ID);
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
    shapeDef.friction             = shapeSettings.Friction;
    shapeDef.restitution          = shapeSettings.Restitution;
    shapeDef.density              = shapeSettings.Density;
    shapeDef.customColor          = static_cast<u32>(shapeSettings.CustomColor.R << 16 | shapeSettings.CustomColor.G << 8 | shapeSettings.CustomColor.B);
    shapeDef.isSensor             = shapeSettings.IsSensor;
    shapeDef.enableSensorEvents   = shapeSettings.EnableSensorEvents;
    shapeDef.enableContactEvents  = shapeSettings.EnableContactEvents;
    shapeDef.enableHitEvents      = shapeSettings.EnableHitEvents;
    shapeDef.enablePreSolveEvents = shapeSettings.EnablePreSolveEvents;
    shapeDef.forceContactCreation = shapeSettings.ForceContactCreation;
    return shapeDef;
}

b2d_shape::b2d_shape(b2d_body* body, polygon_shape::settings const& shapeSettings)
{
    std::vector<b2Vec2> v;
    v.reserve(shapeSettings.Verts.size());
    for (auto const& vert : shapeSettings.Verts) {
        v.emplace_back(vert.X, vert.Y);
    }

    b2Hull h {b2ComputeHull(v.data(), static_cast<i32>(v.size()))};
    auto   poly {b2MakePolygon(&h, shapeSettings.Radius)};

    b2ShapeDef shapeDef {GetShapeDef(shapeSettings)};
    ID = b2CreatePolygonShape(body->ID, &shapeDef, &poly);
}

b2d_shape::b2d_shape(b2d_body* body, rect_shape::settings const& shapeSettings)
{
    auto const& rect {shapeSettings.Extents};
    auto        poly {b2MakeOffsetBox(rect.width() / 2, rect.height() / 2, to_b2Vec2(rect.top_left()), shapeSettings.Angle.Value)};

    b2ShapeDef shapeDef {GetShapeDef(shapeSettings)};
    ID = b2CreatePolygonShape(body->ID, &shapeDef, &poly);
}

b2d_shape::b2d_shape(b2d_body* body, circle_shape::settings const& shapeSettings)
{
    b2Circle poly {to_b2Vec2(shapeSettings.Center), shapeSettings.Radius};

    b2ShapeDef shapeDef {GetShapeDef(shapeSettings)};
    ID = b2CreateCircleShape(body->ID, &shapeDef, &poly);
}

b2d_shape::b2d_shape(b2d_body* body, segment_shape::settings const& shapeSettings)
{
    b2Segment poly {to_b2Vec2(shapeSettings.Point0), to_b2Vec2(shapeSettings.Point1)};

    b2ShapeDef shapeDef {GetShapeDef(shapeSettings)};
    ID = b2CreateSegmentShape(body->ID, &shapeDef, &poly);
}

b2d_shape::b2d_shape(b2d_body* body, capsule_shape::settings const& shapeSettings)
{
    b2Capsule poly {to_b2Vec2(shapeSettings.Center0), to_b2Vec2(shapeSettings.Center1), shapeSettings.Radius};

    b2ShapeDef shapeDef {GetShapeDef(shapeSettings)};
    ID = b2CreateCapsuleShape(body->ID, &shapeDef, &poly);
}

b2d_shape::~b2d_shape()
{
    b2DestroyShape(ID);
}

auto b2d_shape::is_sensor() const -> bool
{
    return b2Shape_IsSensor(ID);
}

void b2d_shape::set_density(f32 density) const
{
    b2Shape_SetDensity(ID, density);
}

auto b2d_shape::get_density() const -> f32
{
    return b2Shape_GetDensity(ID);
}

void b2d_shape::set_friction(f32 friction) const
{
    b2Shape_SetFriction(ID, friction);
}

auto b2d_shape::get_friction() const -> f32
{
    return b2Shape_GetFriction(ID);
}

void b2d_shape::set_restitution(f32 restitution) const
{
    b2Shape_SetRestitution(ID, restitution);
}

auto b2d_shape::get_restitution() const -> f32
{
    return b2Shape_GetRestitution(ID);
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
    auto const val {b2Shape_GetClosestPoint(ID, to_b2Vec2(target))};
    return {val.x, val.y};
}

void b2d_shape::set_user_data(void* ptr) const
{
    b2Shape_SetUserData(ID, ptr);
}

auto b2d_shape::equal(b2d_shape const* other) const -> bool
{
    return B2_ID_EQUALS(ID, other->ID);
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

    ddraw->draw_solid_polygon({{transform.p.x, transform.p.y}, radian_f {b2Rot_GetAngle(transform.q)}}, verts, radius, color::FromRGB(color));
}

void static DrawCircle(b2Vec2 center, float radius, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->draw_circle({center.x, center.y}, radius, color::FromRGB(color));
}

void static DrawSolidCircle(b2Transform transform, float radius, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->draw_solid_circle({{transform.p.x, transform.p.y}, radian_f {b2Rot_GetAngle(transform.q)}}, radius, color::FromRGB(color));
}

void static DrawCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->draw_capsule({p1.x, p1.y}, {p2.x, p2.y}, radius, color::FromRGB(color));
}

void static DrawSolidCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->draw_solid_capsule({p1.x, p1.y}, {p2.x, p2.y}, radius, color::FromRGB(color));
}

void static DrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->draw_segment({p1.x, p1.y}, {p2.x, p2.y}, color::FromRGB(color));
}

void static DrawTransform(b2Transform transform, void* context)
{
    auto* ddraw {reinterpret_cast<b2d_debug_draw*>(context)};
    ddraw->draw_transform({{transform.p.x, transform.p.y}, radian_f {b2Rot_GetAngle(transform.q)}});
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
    ID.DrawCapsule      = &DrawCapsule;
    ID.DrawSolidCapsule = &DrawSolidCapsule;
    ID.DrawSegment      = &DrawSegment;
    ID.DrawTransform    = &DrawTransform;
    ID.DrawPoint        = &DrawPoint;
    ID.DrawString       = &DrawString;
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

void b2d_debug_draw::draw_capsule(point_f p1, point_f p2, f32 radius, color color)
{
    _parent->draw_capsule(p1, p2, radius, color);
}

void b2d_debug_draw::draw_solid_capsule(point_f p1, point_f p2, f32 radius, color color)
{
    _parent->draw_solid_capsule(p1, p2, radius, color);
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
