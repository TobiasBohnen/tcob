// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_CHIPMUNK2D)

    #include <vector>

    #include "tcob/core/Interfaces.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/core/Property.hpp"
    #include "tcob/core/Rect.hpp"
    #include "tcob/physics/chipmunk2d/CP.hpp"

namespace tcob::physics::chipmunk2d {
////////////////////////////////////////////////////////////

class TCOB_API shape : public non_copyable {
    friend auto detail::get_impl(shape const* b) -> cpShape*;

public:
    ~shape();

    prop_fn<f32>       Mass;
    prop_fn<f32>       Density;
    prop_fn<bool>      IsSensor;
    prop_fn<f32>       Elasticity;
    prop_fn<f32>       Friction;
    prop_fn<point_f>   SurfaceVelocity;
    prop_fn<uintptr_t> CollisionType;
    // frw_property<cpShapeFilter> Filter;

    auto get_moment() const -> f32;
    auto get_area() const -> f32;
    auto get_center_of_gravity() const -> point_f;
    auto get_bounding_box() const -> rect_f;

protected:
    shape(cpShape* shape);

private:
    cpShape* _cpShape;
};

////////////////////////////////////////////////////////////

struct circle_shape_settings {
    f32     Radius {};
    point_f Offset;
};

class TCOB_API circle_shape : public shape {
public:
    circle_shape(cpBody* body, f32 radius, point_f offset);
};

////////////////////////////////////////////////////////////

struct segment_shape_settings {
    point_f A;
    point_f B;
    f32     Radius {};
};

class TCOB_API segment_shape : public shape {
public:
    segment_shape(cpBody* body, point_f a, point_f b, f32 radius);
};

////////////////////////////////////////////////////////////

struct poly_shape_settings {
    std::vector<point_f> Verts;
    f32                  Radius;
};

class TCOB_API poly_shape : public shape {
public:
    poly_shape(cpBody* body, std::vector<point_f> const& verts, f32 radius);
};

////////////////////////////////////////////////////////////

struct box_shape_settings {
    size_f Size;
    f32    Radius {};
};

struct box_shape_settings2 {
    rect_f Box;
    f32    Radius {};
};

class TCOB_API box_shape : public shape {
public:
    box_shape(cpBody* body, size_f size, f32 radius);
    box_shape(cpBody* body, rect_f const& box, f32 radius);
};

}

#endif
