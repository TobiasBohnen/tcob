// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/B2DShape.hpp"

#include <memory>
#include <utility>
#include <vector>

#include "B2D.hpp"

#include "tcob/core/Point.hpp"
#include "tcob/physics/B2DBody.hpp"
#include "tcob/physics/Physics.hpp"

namespace tcob::physics {

////////////////////////////////////////////////////////////

auto shape::get_impl() -> detail::b2d_shape*
{
    return _impl.get();
}

shape::shape(body& body, std::unique_ptr<detail::b2d_shape> impl)
    : Friction {detail::make_prop<f32, &detail::b2d_shape::get_friction, &detail::b2d_shape::set_friction>(this)}
    , Restitution {detail::make_prop<f32, &detail::b2d_shape::get_restitution, &detail::b2d_shape::set_restitution>(this)}
    , Density {detail::make_prop<f32, &detail::b2d_shape::get_density, &detail::b2d_shape::set_density>(this)}
    , _impl {std::move(impl)}
    , _body {body}
{
    _impl->set_user_data(this);
}

shape::~shape() = default;

auto shape::operator==(shape const& other) const -> bool
{
    return _impl->equal(other._impl.get());
}

auto shape::parent() -> body&
{
    return _body;
}

auto shape::aabb() const -> AABB
{
    return _impl->get_aabb();
}

auto shape::mass_data() const -> physics::mass_data
{
    return _impl->get_mass_data();
}

auto shape::is_sensor() const -> bool
{
    return _impl->is_sensor();
}

auto shape::sensor_overlaps() const -> std::vector<shape*>
{
    return _impl->get_sensor_overlaps();
}

auto shape::test_point(point_f point) const -> bool
{
    return _impl->test_point(point);
}

auto shape::get_closest_point(point_f target) const -> point_f
{
    return _impl->get_closest_point(target);
}

void shape::enable_sensor_events(bool enable) const
{
    _impl->enable_sensor_events(enable);
}

void shape::enable_contact_events(bool enable) const
{
    _impl->enable_contact_events(enable);
}

void shape::enable_hit_events(bool enable) const
{
    _impl->enable_hit_events(enable);
}

void shape::enable_pre_solve_events(bool enable) const
{
    _impl->enable_pre_solve_events(enable);
}

////////////////////////////////////////////////////////////

polygon_shape::polygon_shape(body& body, detail::b2d_body* b2dBody, settings const& settings, shape::settings const& shapeSettings)
    : shape {body, std::make_unique<detail::b2d_shape>(b2dBody, settings, shapeSettings)}
{
}

rect_shape::rect_shape(body& body, detail::b2d_body* b2dBody, settings const& settings, shape::settings const& shapeSettings)
    : shape {body, std::make_unique<detail::b2d_shape>(b2dBody, settings, shapeSettings)}
{
}

circle_shape::circle_shape(body& body, detail::b2d_body* b2dBody, settings const& settings, shape::settings const& shapeSettings)
    : shape {body, std::make_unique<detail::b2d_shape>(b2dBody, settings, shapeSettings)}
{
}

segment_shape::segment_shape(body& body, detail::b2d_body* b2dBody, settings const& settings, shape::settings const& shapeSettings)
    : shape {body, std::make_unique<detail::b2d_shape>(b2dBody, settings, shapeSettings)}
{
}

capsule_shape::capsule_shape(body& body, detail::b2d_body* b2dBody, settings const& settings, shape::settings const& shapeSettings)
    : shape {body, std::make_unique<detail::b2d_shape>(b2dBody, settings, shapeSettings)}
{
}

////////////////////////////////////////////////////////////

auto chain::get_impl() -> detail::b2d_chain*
{
    return _impl.get();
}

chain::chain(body& body, detail::b2d_body* b2dBody, settings const& settings)
    : Friction {detail::make_prop<f32, &detail::b2d_chain::get_friction, &detail::b2d_chain::set_friction>(this)}
    , Restitution {detail::make_prop<f32, &detail::b2d_chain::get_restitution, &detail::b2d_chain::set_restitution>(this)}
    , _impl {std::make_unique<detail::b2d_chain>(b2dBody, settings)}
    , _body {body}
{
}

chain::~chain() = default;

auto chain::parent() -> body&
{
    return _body;
}

auto chain::segments() -> std::vector<chain_segment>
{
    return _impl->get_segments();
}

}
