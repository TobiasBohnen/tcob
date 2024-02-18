// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/physics/box2d/B2DWorld.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <box2d/box2d.h>

namespace tcob::physics::box2d::detail {

class contact_listener : public b2ContactListener {
public:
    contact_listener(world* world)
        : _world {world}
    {
    }

    void BeginContact(b2Contact* contact) override;

    void EndContact(b2Contact* contact) override;

    void PreSolve(b2Contact* contact, b2Manifold const* oldManifold) override;

    void PostSolve(b2Contact* contact, b2ContactImpulse const* impulse) override;

private:
    auto convert_contact(b2Contact* contact) const -> contact_event;

    world* _world;
};
}

#endif
