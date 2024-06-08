// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/chipmunk2d/CPShape.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_CHIPMUNK2D)

    #include <chipmunk/chipmunk.h>

namespace tcob::physics::chipmunk2d {

shape::shape(cpShape* shape)
    : Mass {{[&]() { return cpShapeGetMass(_cpShape); },
             [&](auto const& value) { cpShapeSetMass(_cpShape, value); }}}
    , Density {{[&]() { return cpShapeGetDensity(_cpShape); },
                [&](auto const& value) { cpShapeSetDensity(_cpShape, value); }}}
    , IsSensor {{[&]() { return cpShapeGetSensor(_cpShape); },
                 [&](auto const& value) { cpShapeSetSensor(_cpShape, value); }}}
    , Elasticity {{[&]() { return cpShapeGetElasticity(_cpShape); },
                   [&](auto const& value) { cpShapeSetElasticity(_cpShape, value); }}}
    , Friction {{[&]() { return cpShapeGetFriction(_cpShape); },
                 [&](auto const& value) { cpShapeSetFriction(_cpShape, value); }}}
    , SurfaceVelocity {{[&]() { return detail::to_point(cpShapeGetSurfaceVelocity(_cpShape)); },
                        [&](auto const& value) { cpShapeSetSurfaceVelocity(_cpShape, {value.X, value.Y}); }}}
    , CollisionType {{[&]() { return cpShapeGetCollisionType(_cpShape); },
                      [&](auto const& value) { cpShapeSetCollisionType(_cpShape, value); }}}
    , _cpShape {shape}
{
    cpSpaceAddShape(cpBodyGetSpace(cpShapeGetBody(shape)), shape);
}

shape::~shape()
{
    if (_cpShape) {
        cpShapeFree(_cpShape);
        _cpShape = nullptr;
    }
}

auto shape::get_moment() const -> f32
{
    return cpShapeGetMoment(_cpShape);
}

auto shape::get_area() const -> f32
{
    return cpShapeGetArea(_cpShape);
}

auto shape::get_center_of_gravity() const -> point_f
{
    return detail::to_point(cpShapeGetCenterOfGravity(_cpShape));
}

auto shape::get_bounding_box() const -> rect_f
{
    return detail::to_rect(cpShapeGetBB(_cpShape));
}

////////////////////////////////////////////////////////////

circle_shape::circle_shape(cpBody* body, f32 radius, point_f offset)
    : shape {cpCircleShapeNew(body, radius, {offset.X, offset.Y})}
{
}

////////////////////////////////////////////////////////////

segment_shape::segment_shape(cpBody* body, point_f a, point_f b, f32 radius)
    : shape {cpSegmentShapeNew(body, {a.X, a.Y}, {b.X, b.Y}, radius)}
{
}

////////////////////////////////////////////////////////////

poly_shape::poly_shape(cpBody* body, std::vector<point_f> const& verts, f32 radius)
    : shape {cpPolyShapeNew(body, static_cast<i32>(verts.size()), reinterpret_cast<cpVect const*>(verts.data()), cpTransformIdentity, radius)}
{
}

////////////////////////////////////////////////////////////

box_shape::box_shape(cpBody* body, size_f size, f32 radius)
    : shape {cpBoxShapeNew(body, size.Width, size.Height, radius)}
{
}

////////////////////////////////////////////////////////////

box_shape::box_shape(cpBody* body, rect_f const& box, f32 radius)
    : shape {cpBoxShapeNew2(body, {box.left(), box.top(), box.right(), box.bottom()}, radius)}
{
}

} // namespace chipmunk2d

#endif
