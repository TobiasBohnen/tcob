// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include "tcob/core/Interfaces.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/core/Property.hpp"
    #include "tcob/physics/box2d/B2D.hpp"

namespace tcob::physics::box2d {
////////////////////////////////////////////////////////////

struct fixture_settings {

    /// The friction coefficient, usually in the range [0,1].
    f32 Friction {0.2f};

    /// The restitution (elasticity) usually in the range [0,1].
    f32 Restitution {0.01f};

    /// Restitution velocity threshold, usually in m/s. Collisions above this
    /// speed have restitution applied (will bounce).
    f32 RestitutionThreshold {1.0f};

    /// The density, usually in kg/m^2.
    f32 Density {0.0f};

    /// A sensor shape collects contact information but never generates a collision
    /// response.
    bool IsSensor {false};

    /// Contact filtering data.
    //  b2Filter filter;
};

class TCOB_API fixture final : public non_copyable {
    friend auto detail::get_impl(fixture const* b) -> b2Fixture*;
    friend class body;

public:
    prop_fn<f32>   Friction;
    prop_fn<f32>   Restitution;
    prop_fn<f32>   RestitutionThreshold;
    prop_fn<f32>   Density;
    prop_fn<bool>  IsSensor;
    prop<std::any> UserData;

    auto get_body() -> body&;

    auto test_point(point_f point) -> bool;

private:
    fixture(b2Fixture* fix, body* parent);

    b2Fixture* _b2Fixture;
    body*      _body;
};

inline auto operator==(fixture const& left, fixture const& right) -> bool
{
    return detail::get_impl(&left) == detail::get_impl(&right);
}

}

#endif
