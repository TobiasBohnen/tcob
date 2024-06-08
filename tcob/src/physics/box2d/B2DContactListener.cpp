// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "B2DContactListener.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

namespace tcob::physics::box2d::detail {

void contact_listener::BeginContact(b2Contact* contact)
{
    contact_event ev {convert_contact(contact)};
    _world->BeginContact(ev);
    contact->SetEnabled(ev.Enabled);
    contact->SetFriction(ev.Friction);
    contact->SetRestitution(ev.Restitution);
}

void contact_listener::EndContact(b2Contact* contact)
{
    contact_event ev {convert_contact(contact)};
    _world->EndContact(ev);
    contact->SetEnabled(ev.Enabled);
    contact->SetFriction(ev.Friction);
    contact->SetRestitution(ev.Restitution);
}

void contact_listener::PreSolve(b2Contact* contact, b2Manifold const* /* oldManifold */)
{
    contact_event ev {convert_contact(contact)};
    _world->PreSolve(ev);
    contact->SetEnabled(ev.Enabled);
    contact->SetFriction(ev.Friction);
    contact->SetRestitution(ev.Restitution);
}

void contact_listener::PostSolve(b2Contact* contact, b2ContactImpulse const* /* impulse */)
{
    contact_event ev {convert_contact(contact)};
    _world->PostSolve(ev);
    contact->SetEnabled(ev.Enabled);
    contact->SetFriction(ev.Friction);
    contact->SetRestitution(ev.Restitution);
}

auto contact_listener::convert_contact(b2Contact* contact) const -> contact_event
{
    return {.FixtureA    = _world->find_fixture(contact->GetFixtureA()),
            .FixtureB    = _world->find_fixture(contact->GetFixtureB()),
            .IsTouching  = contact->IsTouching(),
            .Enabled     = contact->IsEnabled(),
            .Friction    = contact->GetFriction(),
            .Restitution = contact->GetRestitution()};
}

} // namespace box2d

#endif
