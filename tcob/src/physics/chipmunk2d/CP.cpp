// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/chipmunk2d/CP.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_CHIPMUNK2D)

    #include <chipmunk/chipmunk.h>

    #include "tcob/physics/chipmunk2d/CPBody.hpp"
    #include "tcob/physics/chipmunk2d/CPSpace.hpp"

namespace tcob::physics::chipmunk2d {

auto detail::find_body(space* s, cpBody* cpbody) -> std::shared_ptr<body>
{
    for (auto& body : s->_bodies) {
        if (detail::get_impl(body.get()) == cpbody) {
            return body;
        }
    }

    return nullptr;
}

auto detail::find_shape(body* s, cpShape* cpshape) -> std::shared_ptr<shape>
{
    for (auto& shape : s->_shapes) {
        if (detail::get_impl(shape.get()) == cpshape) {
            return shape;
        }
    }

    return nullptr;
}

auto static get_contact_event(cpArbiter* arb, space* sp) -> contact_event
{
    cpBody* a {};
    cpBody* b {};
    cpArbiterGetBodies(arb, &a, &b);

    cpShape* aa {};
    cpShape* bb {};
    cpArbiterGetShapes(arb, &aa, &bb);

    contact_event ev;
    ev.BodyA          = detail::find_body(sp, a);
    ev.BodyB          = detail::find_body(sp, b);
    ev.ShapeA         = detail::find_shape(ev.BodyA.get(), aa);
    ev.ShapeB         = detail::find_shape(ev.BodyB.get(), bb);
    ev.IsFirstContact = (cpArbiterIsFirstContact(arb) != 0u);
    ev.IsRemoval      = (cpArbiterIsRemoval(arb) != 0u);
    return ev;
}

auto detail::begin_func(cpArbiter* arb, cpSpace*, void* userData) -> unsigned char
{
    space* sp {static_cast<space*>(userData)};
    sp->BeginContact(get_contact_event(arb, sp));

    return 1u;
}

void detail::separate_func(cpArbiter* arb, cpSpace*, void* userData)
{
    space* sp {static_cast<space*>(userData)};
    sp->EndContact(get_contact_event(arb, sp));
}

auto detail::to_point(cpVect const& v) -> point_f
{
    return {v.x, v.y};
}

auto detail::to_rect(cpBB const& v) -> rect_f
{
    return {v.l, v.b, v.r, v.t};
}

auto detail::get_impl(space const* b) -> cpSpace*
{
    return b->_cpSpace;
}

auto detail::get_impl(body const* b) -> cpBody*
{
    return b->_cpBody;
}

auto detail::get_impl(shape const* b) -> cpShape*
{
    return b->_cpShape;
}

auto detail::get_impl(constraint const* b) -> cpConstraint*
{
    return b->_cpConstraint;
}

auto moment_for_circle(f32 m, f32 r1, f32 r2, point_f offset) -> f32
{
    return cpMomentForCircle(m, r1, r2, {offset.X, offset.Y});
}

auto moment_for_segment(f32 m, point_f a, point_f b, f32 radius) -> f32
{
    return cpMomentForSegment(m, {a.X, a.Y}, {b.X, b.Y}, radius);
}

auto moment_for_poly(f32 m, std::span<point_f const> verts, point_f offset, f32 radius) -> f32
{
    return cpMomentForPoly(m, static_cast<i32>(verts.size()), reinterpret_cast<cpVect const*>(verts.data()), {offset.X, offset.Y}, radius);
}

auto moment_for_box(f32 m, f32 width, f32 height) -> f32
{
    return cpMomentForBox(m, width, height);
}

auto moment_for_box(f32 m, rect_f const& box) -> f32
{
    return cpMomentForBox2(m, {box.left(), box.top(), box.right(), box.bottom()});
}

} // namespace chipmunk2d

#endif
