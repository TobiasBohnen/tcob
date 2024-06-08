// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/box2d/B2DFixture.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <box2d/box2d.h>

namespace tcob::physics::box2d {

fixture::fixture(b2Fixture* fix, body* parent)
    : Friction {{[&]() { return _b2Fixture->GetFriction(); },
                 [&](auto const& value) { _b2Fixture->SetFriction(value); }}}
    , Restitution {{[&]() { return _b2Fixture->GetRestitution(); },
                    [&](auto const& value) { _b2Fixture->SetRestitution(value); }}}
    , RestitutionThreshold {{[&]() { return _b2Fixture->GetRestitutionThreshold(); },
                             [&](auto const& value) { _b2Fixture->SetRestitutionThreshold(value); }}}
    , Density {{[&]() { return _b2Fixture->GetDensity(); },
                [&](auto const& value) { _b2Fixture->SetDensity(value); }}}
    , IsSensor {{[&]() { return _b2Fixture->IsSensor(); },
                 [&](auto const& value) { _b2Fixture->SetSensor(value); }}}
    , _b2Fixture {fix}
    , _body {parent}
{
}

auto fixture::get_body() -> body&
{
    return *_body;
}

auto fixture::test_point(point_f point) -> bool
{
    assert(_b2Fixture);
    return _b2Fixture->TestPoint({point.X, point.Y});
}
}

#endif
