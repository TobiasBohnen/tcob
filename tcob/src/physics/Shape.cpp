// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/Shape.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include "B2D.hpp"

namespace tcob::physics {

////////////////////////////////////////////////////////////

auto shape::operator==(shape const& other) const -> bool
{
    return _impl->equal(other._impl.get());
}

shape::shape(std::unique_ptr<detail::b2d_shape> impl)
    : _impl {std::move(impl)}
{
}

shape::~shape() = default;

polygon_shape::polygon_shape(detail::b2d_body* body, polygon_shape_settings const& shapeSettings)
    : shape {std::make_unique<detail::b2d_shape>(body, shapeSettings)}
{
}

rect_shape::rect_shape(detail::b2d_body* body, rect_shape_settings const& shapeSettings)
    : shape {std::make_unique<detail::b2d_shape>(body, shapeSettings)}
{
}

circle_shape::circle_shape(detail::b2d_body* body, circle_shape_settings const& shapeSettings)
    : shape {std::make_unique<detail::b2d_shape>(body, shapeSettings)}
{
}

segment_shape::segment_shape(detail::b2d_body* body, segment_shape_settings const& shapeSettings)
    : shape {std::make_unique<detail::b2d_shape>(body, shapeSettings)}
{
}

}

#endif
