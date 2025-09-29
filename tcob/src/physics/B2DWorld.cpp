// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/B2DWorld.hpp"

#include <memory>
#include <span>

#include <B2D.hpp>

#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/physics/B2DBody.hpp"
#include "tcob/physics/B2DDebugDraw.hpp"
#include "tcob/physics/B2DJoint.hpp"
#include "tcob/physics/B2DShape.hpp"
#include "tcob/physics/Physics.hpp"

namespace tcob::physics {

auto world::get_impl() -> detail::b2d_world*
{
    return _impl.get();
}

world::world()
    : world {settings {}}
{
}

world::world(settings const& settings)
    : Gravity {detail::make_prop<point_f, &detail::b2d_world::get_gravity, &detail::b2d_world::set_gravity>(this)}
    , RestitutionThreshold {detail::make_prop<f32, &detail::b2d_world::get_restitution_threshold, &detail::b2d_world::set_restitution_threshold>(this)}
    , HitEventThreshold {detail::make_prop<f32, &detail::b2d_world::get_hit_event_threshold, &detail::b2d_world::set_hit_event_threshold>(this)}
    , MaximumLinearSpeed {detail::make_prop<f32, &detail::b2d_world::get_maximum_linear_speed, &detail::b2d_world::set_maximum_linear_speed>(this)}
    , EnableSleeping {detail::make_prop<bool, &detail::b2d_world::get_enable_sleeping, &detail::b2d_world::set_enable_sleeping>(this)}
    , _impl {std::make_unique<detail::b2d_world>(settings)}
{
}

world::~world() = default;

auto world::bodies() -> std::span<std::shared_ptr<body>>
{
    return _bodies;
}

auto world::joints() -> std::span<std::shared_ptr<joint>>
{
    return _joints;
}

auto world::create_body(body_transform const& xform, body::settings const& bodySettings) -> std::shared_ptr<body>
{
    return _bodies.emplace_back(std::shared_ptr<body> {new body {*this, _impl.get(), xform, bodySettings}});
}

void world::remove_body(body const& body)
{
    helper::erase_first(_bodies, [ptr = &body](auto const& val) { return val.get() == ptr; });
}

auto world::find_body(shape const& s) -> std::shared_ptr<body>
{
    for (auto& body : _bodies) {
        for (auto& shape : body->shapes()) {
            if (*shape == s) {
                return body;
            }
        }
    }

    return nullptr;
}

auto world::awake_body_count() const -> i32
{
    return _impl->get_awake_body_count();
}

void world::remove_joint(joint const& joint)
{
    helper::erase_first(_joints, [ptr = &joint](auto const& val) { return val.get() == ptr; });
}

auto world::body_events() const -> physics::body_events
{
    return _impl->get_body_events();
}

auto world::contact_events() const -> physics::contact_events
{
    return _impl->get_contact_events();
}

auto world::sensor_events() const -> physics::sensor_events
{
    return _impl->get_sensor_events();
}

void world::draw(debug_draw const& draw) const
{
    _impl->draw(draw._impl.get(), draw.Settings);
}

void world::explode(explosion const& explosion) const
{
    _impl->explode(explosion);
}

void world::set_contact_tuning(f32 hertz, f32 damping, f32 pushSpeed) const
{
    _impl->set_contact_tuning(hertz, damping, pushSpeed);
}

void world::on_update(milliseconds deltaTime)
{
    _impl->step(static_cast<f32>(deltaTime.count() / 1000), SubSteps);
}

}
