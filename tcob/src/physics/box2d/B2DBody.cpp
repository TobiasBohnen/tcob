// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/box2d/B2DBody.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <cassert>

    #include <box2d/box2d.h>

    #include "tcob/physics/box2d/B2DWorld.hpp"

namespace tcob::physics::box2d {

body::body(b2Body* body, world* parent)
    : Type {{[&]() {
                 switch (_b2Body->GetType()) {
                 case b2_staticBody:
                     return body_type::Static;
                 case b2_kinematicBody:
                     return body_type::Kinematic;
                 case b2_dynamicBody:
                     return body_type::Dynamic;
                 }
                 return body_type {};
             },
             [&](auto const& value) {
                switch (value) {
                case body_type::Dynamic:
                    _b2Body->SetType(b2_dynamicBody);
                    break;
                case body_type::Static:
                    _b2Body->SetType(b2_staticBody);
                    break;
                case body_type::Kinematic:
                    _b2Body->SetType(b2_kinematicBody);
                    break;
                } }}}
    , LinearVelocity {{[&]() -> point_f { return {_b2Body->GetLinearVelocity().x, _b2Body->GetLinearVelocity().y}; },
                       [&](auto const& value) { _b2Body->SetLinearVelocity({value.X, value.Y}); }}}
    , AngularVelocity {{[&]() -> radian_f { return radian_f {_b2Body->GetAngularVelocity()}; },
                        [&](auto const& value) { _b2Body->SetAngularVelocity(value.Value); }}}
    , LinearDamping {{[&]() -> f32 { return _b2Body->GetLinearDamping(); },
                      [&](auto const& value) { _b2Body->SetLinearDamping(value); }}}
    , AngularDamping {{[&]() -> f32 { return _b2Body->GetAngularDamping(); },
                       [&](auto const& value) { _b2Body->SetAngularDamping(value); }}}
    , AllowSleep {{[&]() -> bool { return _b2Body->IsSleepingAllowed(); },
                   [&](auto const& value) { _b2Body->SetSleepingAllowed(value); }}}
    , Awake {{[&]() -> bool { return _b2Body->IsAwake(); },
              [&](auto const& value) { _b2Body->SetAwake(value); }}}
    , IsFixedRotation {{[&]() -> bool { return _b2Body->IsFixedRotation(); },
                        [&](auto const& value) { _b2Body->SetFixedRotation(value); }}}
    , IsBullet {{[&]() -> bool { return _b2Body->IsBullet(); },
                 [&](auto const& value) { _b2Body->SetBullet(value); }}}
    , Enabled {{[&]() -> bool { return _b2Body->IsEnabled(); },
                [&](auto const& value) { _b2Body->SetEnabled(value); }}}
    , GravityScale {{[&]() { return _b2Body->GetGravityScale(); },
                     [&](auto const& value) { _b2Body->SetGravityScale(value); }}}
    , Transform {{[&]() -> body_transform { return {.Position = {_b2Body->GetPosition().x, _b2Body->GetPosition().y},
                                                    .Angle    = radian_f {_b2Body->GetAngle()}}; },
                  [&](auto const& value) { _b2Body->SetTransform({value.Position.X, value.Position.Y}, value.Angle.Value); }}}
    , _b2Body {body}
    , _world {parent}
{
}

void body::apply_force(point_f force, point_f point, bool wake) const
{
    assert(_b2Body);
    _b2Body->ApplyForce({force.X, force.Y}, {point.X, point.Y}, wake);
}

void body::apply_force_to_center(point_f force, bool wake) const
{
    assert(_b2Body);
    _b2Body->ApplyForceToCenter({force.X, force.Y}, wake);
}

void body::apply_linear_impulse(point_f imp, point_f point, bool wake) const
{
    assert(_b2Body);
    _b2Body->ApplyLinearImpulse({imp.X, imp.Y}, {point.X, point.Y}, wake);
}

void body::apply_linear_impulse_to_center(point_f imp, bool wake) const
{
    assert(_b2Body);
    _b2Body->ApplyLinearImpulseToCenter({imp.X, imp.Y}, wake);
}

void body::apply_torgue(f32 torgue, bool wake) const
{
    assert(_b2Body);
    _b2Body->ApplyTorque(torgue, wake);
}

void body::apply_angular_impulse(f32 impulse, bool wake) const
{
    assert(_b2Body);
    _b2Body->ApplyAngularImpulse(impulse, wake);
}

auto body::get_center() const -> point_f
{
    auto vec {_b2Body->GetWorldCenter()};
    return {vec.x, vec.y};
}

auto body::get_local_center() const -> point_f
{
    auto vec {_b2Body->GetLocalCenter()};
    return {vec.x, vec.y};
}

auto body::get_world() const -> world&
{
    return *_world;
}

auto body::get_fixtures() -> std::span<std::shared_ptr<fixture>>
{
    return _fixtures;
}

auto body::create_fixture(shape const& shape, fixture_settings const& settings) -> std::shared_ptr<fixture>
{
    assert(_b2Body);

    b2FixtureDef def;
    def.density              = settings.Density;
    def.friction             = settings.Friction;
    def.isSensor             = settings.IsSensor;
    def.restitution          = settings.Restitution;
    def.restitutionThreshold = settings.RestitutionThreshold;
    def.shape                = detail::get_impl(&shape);

    auto* fixPtr {_b2Body->CreateFixture(&def)};
    auto  retValue {std::shared_ptr<fixture> {new fixture {fixPtr, this}}};
    _fixtures.push_back(retValue);

    return retValue;
}

void body::destroy_fixture(std::shared_ptr<fixture> const& fixturePtr)
{
    _fixtures.erase(std::find(_fixtures.begin(), _fixtures.end(), fixturePtr));
    _b2Body->DestroyFixture(detail::get_impl(fixturePtr.get()));
}

void body::wake_up() const
{
    assert(_b2Body);
    _b2Body->SetAwake(true);
}

void body::sleep() const
{
    assert(_b2Body);
    _b2Body->SetAwake(false);
}
}

#endif
