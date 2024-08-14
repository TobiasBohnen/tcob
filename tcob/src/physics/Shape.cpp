// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/Shape.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include "B2D.hpp"

namespace tcob::physics {

////////////////////////////////////////////////////////////

shape::shape(std::unique_ptr<detail::b2dShape> impl)
    : _impl {std::move(impl)}
{
}

shape::~shape() = default;

polygon_shape::polygon_shape(detail::b2dBody* body, polygon_shape_settings const& shapeSettings)
    : shape {std::make_unique<detail::b2dShape>(body, shapeSettings)}
{
}

rect_shape::rect_shape(detail::b2dBody* body, rect_shape_settings const& shapeSettings)
    : shape {std::make_unique<detail::b2dShape>(body, shapeSettings)}
{
}

circle_shape::circle_shape(detail::b2dBody* body, circle_shape_settings const& shapeSettings)
    : shape {std::make_unique<detail::b2dShape>(body, shapeSettings)}
{
}

segment_shape::segment_shape(detail::b2dBody* body, segment_shape_settings const& shapeSettings)
    : shape {std::make_unique<detail::b2dShape>(body, shapeSettings)}
{
}

}

#endif
