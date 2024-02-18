// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/box2d/B2D.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include "tcob/physics/box2d/B2DBody.hpp"
    #include "tcob/physics/box2d/B2DDebugDraw.hpp"
    #include "tcob/physics/box2d/B2DFixture.hpp"
    #include "tcob/physics/box2d/B2DJoint.hpp"
    #include "tcob/physics/box2d/B2DShape.hpp"

namespace tcob::physics::box2d {

auto detail::get_impl(body const* b) -> b2Body*
{
    return b->_b2Body;
}
auto detail::get_impl(shape const* b) -> b2Shape*
{
    return b->_shape.get();
}
auto detail::get_impl(fixture const* b) -> b2Fixture*
{
    return b->_b2Fixture;
}
auto detail::get_impl(joint const* b) -> b2Joint*
{
    return b->_b2Joint;
}
auto detail::get_impl(debug_draw const* b) -> b2Draw*
{
    return b->_b2Draw;
}

}

#endif
