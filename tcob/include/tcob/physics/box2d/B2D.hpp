// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include "tcob/physics/Physics.hpp" // IWYU pragma: keep

class b2Body;
class b2Fixture;
class b2Shape;
class b2Joint;
class b2World;
class b2Draw;

namespace tcob::physics::box2d {

class body;
class fixture;
class shape;
class joint;
class world;
class debug_draw;

namespace detail {
    class contact_listener;

    TCOB_API auto get_impl(body const* b) -> b2Body*;
    TCOB_API auto get_impl(fixture const* b) -> b2Fixture*;
    TCOB_API auto get_impl(shape const* b) -> b2Shape*;
    TCOB_API auto get_impl(joint const* b) -> b2Joint*;
    TCOB_API auto get_impl(debug_draw const* b) -> b2Draw*;
}

}

#endif
