// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_CHIPMUNK2D)

    #include <span>

    #include "tcob/core/Point.hpp"
    #include "tcob/core/Rect.hpp"

struct cpBody;
struct cpSpace;
struct cpShape;
struct cpConstraint;
struct cpArbiter;
struct cpBB;
struct cpVect;

namespace tcob::physics::chipmunk2d {

class body;
class space;
class shape;
class constraint;

TCOB_API auto moment_for_circle(f32 m, f32 r1, f32 r2, point_f offset) -> f32;
TCOB_API auto moment_for_segment(f32 m, point_f a, point_f b, f32 radius) -> f32;
TCOB_API auto moment_for_poly(f32 m, std::span<point_f const> verts, point_f offset, f32 radius) -> f32;
TCOB_API auto moment_for_box(f32 m, f32 width, f32 height) -> f32;
TCOB_API auto moment_for_box(f32 m, rect_f const& box) -> f32;

namespace detail {
    TCOB_API auto to_point(cpVect const& v) -> point_f;
    TCOB_API auto to_rect(cpBB const& v) -> rect_f;

    TCOB_API auto get_impl(space const* b) -> cpSpace*;
    TCOB_API auto get_impl(body const* b) -> cpBody*;
    TCOB_API auto get_impl(shape const* b) -> cpShape*;
    TCOB_API auto get_impl(constraint const* b) -> cpConstraint*;

    TCOB_API auto find_body(space* s, cpBody* cpbody) -> std::shared_ptr<body>;
    TCOB_API auto find_shape(body* s, cpShape* cpshape) -> std::shared_ptr<shape>;

    TCOB_API auto begin_func(cpArbiter* arb, cpSpace*, void* userData) -> unsigned char;
    TCOB_API void separate_func(cpArbiter* arb, cpSpace*, void* userData);
}

}

#endif
