// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/Shape.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include "B2D.hpp"

namespace tcob::physics {

////////////////////////////////////////////////////////////

shape::shape(body& body, std::unique_ptr<detail::b2d_shape> impl)
    : _impl {std::move(impl)}
    , _body {body}
{
    _impl->set_user_data(this);
}

shape::~shape() = default;

auto shape::operator==(shape const& other) const -> bool
{
    return _impl->equal(other._impl.get());
}

auto shape::get_body() -> body&
{
    return _body;
}

////////////////////////////////////////////////////////////

polygon_shape::polygon_shape(body& body, settings const& shapeSettings)
    : shape {body, std::make_unique<detail::b2d_shape>(detail::get_impl(body), shapeSettings)}
{
}

rect_shape::rect_shape(body& body, settings const& shapeSettings)
    : shape {body, std::make_unique<detail::b2d_shape>(detail::get_impl(body), shapeSettings)}
{
}

circle_shape::circle_shape(body& body, settings const& shapeSettings)
    : shape {body, std::make_unique<detail::b2d_shape>(detail::get_impl(body), shapeSettings)}
{
}

segment_shape::segment_shape(body& body, settings const& shapeSettings)
    : shape {body, std::make_unique<detail::b2d_shape>(detail::get_impl(body), shapeSettings)}
{
}

capsule_shape::capsule_shape(body& body, settings const& shapeSettings)
    : shape {body, std::make_unique<detail::b2d_shape>(detail::get_impl(body), shapeSettings)}
{
}

}

#endif
