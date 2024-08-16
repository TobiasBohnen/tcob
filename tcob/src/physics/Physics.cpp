// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/Physics.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include "tcob/physics/Body.hpp"
    #include "tcob/physics/Joint.hpp"
    #include "tcob/physics/Shape.hpp"
    #include "tcob/physics/World.hpp"

namespace tcob::physics::detail {

auto get_impl(world const& t) -> b2d_world*
{
    return t._impl.get();
}

auto get_impl(body const& t) -> b2d_body*
{
    return t._impl.get();
}

auto get_impl(shape const& t) -> b2d_shape*
{
    return t._impl.get();
}

auto get_impl(joint const& t) -> b2d_joint*
{
    return t._impl.get();
}
}

#endif
