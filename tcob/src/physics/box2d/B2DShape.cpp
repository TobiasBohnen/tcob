// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/box2d/B2DShape.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <box2d/box2d.h>

namespace tcob::physics::box2d {

////////////////////////////////////////////////////////////

shape::shape(std::unique_ptr<b2Shape> shape)
    : _shape {std::move(shape)}
{
}

shape::~shape() = default;

////////////////////////////////////////////////////////////

polygon_shape::polygon_shape()
    : shape {std::make_unique<b2PolygonShape>()}
{
}

void polygon_shape::set(std::span<point_f const> vecs) const
{
    static_assert(sizeof(point_f) == sizeof(b2Vec2));
    get_impl<b2PolygonShape>()->Set(reinterpret_cast<b2Vec2 const*>(vecs.data()), static_cast<i32>(vecs.size()));
}

void polygon_shape::set_as_box(size_f extents) const
{
    get_impl<b2PolygonShape>()->SetAsBox(extents.Width / 2, extents.Height / 2);
}

void polygon_shape::set_as_box(rect_f const& extents, radian_f angle) const
{
    get_impl<b2PolygonShape>()->SetAsBox(extents.Width / 2, extents.Height / 2,
                                         {extents.get_center().X, extents.get_center().Y},
                                         angle.Value);
}

////////////////////////////////////////////////////////////

circle_shape::circle_shape()
    : shape {std::make_unique<b2CircleShape>()}
    , Radius {{[&]() { return get_impl<b2CircleShape>()->m_radius; },
               [&](f32 value) { get_impl<b2CircleShape>()->m_radius = value; }}}
{
}

circle_shape::circle_shape(f32 radius)
    : circle_shape {}
{
    Radius(radius);
}

////////////////////////////////////////////////////////////
edge_shape::edge_shape()
    : shape {std::make_unique<b2EdgeShape>()}
{
}

void edge_shape::set_one_sided(point_f v0, point_f v1, point_f v2, point_f v3)
{
    get_impl<b2EdgeShape>()->SetOneSided({v0.X, v0.Y}, {v1.X, v1.Y}, {v2.X, v2.Y}, {v3.X, v3.Y});
}

void edge_shape::set_two_sided(point_f v1, point_f v2)
{
    get_impl<b2EdgeShape>()->SetTwoSided({v1.X, v1.Y}, {v2.X, v2.Y});
}

}

#endif
